#include "DecodeAndEncode.h"
#include "HammingCode.h"
#include <fstream>
#include <iostream>

const uint8_t kBitsInByte = 8;

IterationTools::IterationTools(size_t block) {
    bits_count = block;
    bytes_count = bits_count / kBitsInByte + (bits_count % kBitsInByte != 0);
}

// Общие функции для обновления параметров у инструментов для итерации по архиву / исходным файлам

void DecodeAndEncode::UpdateTools(size_t default_bits_count_value, IterationTools& tools) {
    if (tools.bytes_count * kBitsInByte - tools.bits_count != 0) {
        tools.buffer_bits = tools.remainder_in_block;
    } else {
        tools.buffer_bits.clear();
    }
    tools.remainder = tools.buffer_bits.size();
    tools.bits_count = default_bits_count_value - tools.remainder;
    tools.bytes_count = tools.bits_count / kBitsInByte + (tools.bits_count % kBitsInByte != 0);
    tools.buffer_bytes.clear();
    tools.remainder_in_block.clear();
    tools.current_symbols_count = 0;
}

void DecodeAndEncode::SetCurrentRemainderInBytes(const std::vector<bool>& bits, std::vector<bool>& remainder_in_bytes) {
    for (size_t i = 0; i < bits.size(); ++i) {
        remainder_in_bytes.push_back(bits[i]);
    }
}

// Обновление остатка, возникающего при переводе закодированных / декодированных битов в байты

void DecodeAndEncode::UpdateRemainderInBytes(std::ofstream& archive, std::vector<bool>& remainder_in_bytes, bool is_it_encode_process, bool is_it_end) {
    if (remainder_in_bytes.size() % kBitsInByte != 0) {
        uint8_t how_much_is_left = remainder_in_bytes.size() % kBitsInByte;
        uint8_t new_remainder[kBitsInByte];

        for (uint8_t i = 0; i < how_much_is_left; ++i) {
            new_remainder[i] = remainder_in_bytes[(remainder_in_bytes.size() / kBitsInByte) * kBitsInByte + i];
        }
        remainder_in_bytes.clear();

        for (uint8_t i = 0; i < how_much_is_left; ++i) {
            remainder_in_bytes.push_back(new_remainder[i]);
        }

        if (is_it_encode_process && is_it_end) {
            uint8_t remember_this_size = remainder_in_bytes.size();
            for (size_t j = 0; j < kBitsInByte - remember_this_size; ++j) {
                remainder_in_bytes.push_back(0); // Добавляем в конец нули в качестве паддинга до размера kBitsInyByte, чтобы поместить в байт (так удобнее декодировать)
            }

            uint8_t info = 0;
            for (uint8_t j = 0; j < kBitsInByte; ++j) {
                info |= (remainder_in_bytes[j] << (kBitsInByte - 1 - j));
            }
            archive << info;
        }
    } else {
        remainder_in_bytes.clear();
    }
}

// Конвертация буфферных байтов в буфферные биты

void DecodeAndEncode::ConvertBufferBytesToBits(IterationTools& tools, bool is_it_decode_process) {
    for (size_t i = 0; i < tools.buffer_bytes.size(); ++i) {
        if (i == tools.buffer_bytes.size() - 1 && (tools.bytes_count * kBitsInByte - tools.bits_count != 0)) {
            for (uint8_t j = 0; j < tools.bits_count % kBitsInByte; ++j) {
                tools.buffer_bits.push_back((tools.buffer_bytes.back() >> (kBitsInByte - 1 - j)) & 1);
            }
            if (is_it_decode_process && tools.is_it_end) {
                continue; // В случае декодирования, поскольку при паддинге мы добавляли нули в конец, их не учитываем, т.е биты в остатке не берем
            }
            for (uint8_t j = tools.bits_count % kBitsInByte; j < kBitsInByte; ++j) {
                tools.remainder_in_block.push_back((tools.buffer_bytes.back() >> (kBitsInByte - 1 - j)) & 1);
            }
        } else {
            for (uint8_t j = 0; j < kBitsInByte; ++j) {
                tools.buffer_bits.push_back((tools.buffer_bytes[i] >> (kBitsInByte - 1 - j)) & 1);
            }
        }
    }
}

// Функциия для декодирования / кодирования блока

void DecodeAndEncode::DecodeBlock(std::vector<bool>& decoded_bits, size_t hamming_block, IterationTools& decode_tools) {
    ConvertBufferBytesToBits(decode_tools, true);
    Hamming::HammingDecode(decode_tools.buffer_bits, decoded_bits, hamming_block);
}

void DecodeAndEncode::EncodeBlock(std::vector<bool>& encoded_bits, size_t hamming_block, IterationTools& encode_tools) {
    ConvertBufferBytesToBits(encode_tools, false);
    Hamming::HammingCode(encode_tools.buffer_bits, encoded_bits, hamming_block);
}

// Добавление декодированной / закодированной информации в соответствующие объекты

void DecodeAndEncode::AddDecodedBitsToString(std::vector<bool>& decoded_bits, std::string& string_with_decoded_bytes, std::vector<bool>& remainder_in_bytes, bool is_it_end) {
    SetCurrentRemainderInBytes(decoded_bits, remainder_in_bytes);

    for (size_t i = 0; i < (remainder_in_bytes.size() / kBitsInByte); ++i) {
        uint8_t info = 0;
        for (uint8_t j = 0; j < kBitsInByte; ++j) {
            info |= (remainder_in_bytes[i * kBitsInByte + j] << (kBitsInByte - 1 - j));
        }
        string_with_decoded_bytes.push_back(info);
    }

    std::ofstream not_archive;
    UpdateRemainderInBytes(not_archive, remainder_in_bytes, false, is_it_end);
}

void DecodeAndEncode::AddDecodedBitsToSizeT(std::vector<bool>& decoded_bits, size_t& object_len, std::vector<bool>& remainder_in_bytes, uint8_t& insertable_idx, bool is_it_end) {
    SetCurrentRemainderInBytes(decoded_bits, remainder_in_bytes);

    for (size_t i = 0; i < (remainder_in_bytes.size() / kBitsInByte); ++i) {
        uint8_t info = 0;
        for (uint8_t j = 0; j < kBitsInByte; ++j) {
            info |= (remainder_in_bytes[i * kBitsInByte + j] << (kBitsInByte - 1 - j));
        }
        object_len |= (info << (kBitsInByte * (kBitsInByte - 1 - insertable_idx)));
        ++insertable_idx;
    }

    std::ofstream not_archive;
    UpdateRemainderInBytes(not_archive, remainder_in_bytes, false, is_it_end);
}

void DecodeAndEncode::AddDecodedBitsToFile(std::vector<bool>& decoded_bits, std::ofstream& file, std::vector<bool>& remainder_in_bytes, bool is_it_end) {
    SetCurrentRemainderInBytes(decoded_bits, remainder_in_bytes);

    for (size_t i = 0; i < (remainder_in_bytes.size() / kBitsInByte); ++i) {
        uint8_t info = 0;
        for (uint8_t j = 0; j < kBitsInByte; ++j) {
            info |= (remainder_in_bytes[i * kBitsInByte + j] << (kBitsInByte - 1 - j));
        }
        file << info;
    }

    std::ofstream not_archive;
    UpdateRemainderInBytes(not_archive, remainder_in_bytes, false, is_it_end);
}

void DecodeAndEncode::AddEncodedBitsToArchive(const std::vector<bool>& encoded_bits, std::ofstream& archive, std::vector<bool>& remainder_in_bytes, bool is_it_end) {
    SetCurrentRemainderInBytes(encoded_bits, remainder_in_bytes);

    for (size_t i = 0; i < (remainder_in_bytes.size() / kBitsInByte); ++i) {
        uint8_t info = 0;
        for (uint8_t j = 0; j < kBitsInByte; ++j) {
            info |= (remainder_in_bytes[i * kBitsInByte + j] << (kBitsInByte - 1 - j));
        }
        archive << info;
    }

    UpdateRemainderInBytes(archive, remainder_in_bytes, true, is_it_end);
}

// Итерирование по исходным файлам / архиву с дальнейшим кодированием / декодированием

void DecodeAndEncode::DecodeBytesAndPutToString(char info, size_t hamming_block, std::string& string_with_decoded_bytes, IterationTools& decode_tools) {
    decode_tools.buffer_bytes.push_back(info);
    ++decode_tools.current_symbols_count;

    if (decode_tools.current_symbols_count == decode_tools.bytes_count) {
        std::vector<bool> decoded_bits;
        DecodeBlock(decoded_bits, hamming_block, decode_tools);

        AddDecodedBitsToString(decoded_bits, string_with_decoded_bytes, decode_tools.remainder_in_bytes, decode_tools.is_it_end);

        UpdateTools(hamming_block + Hamming::CalcControlBits(hamming_block) + 1, decode_tools);
    }
    ++decode_tools.i;
}

void DecodeAndEncode::DecodeBytesAndPutToSizeT(char info, size_t hamming_block, size_t& object_len, IterationTools& decode_tools, uint8_t& insertable_idx) {
    decode_tools.buffer_bytes.push_back(info);
    ++decode_tools.current_symbols_count;

    if (decode_tools.current_symbols_count == decode_tools.bytes_count) {
        std::vector<bool> decoded_bits;
        DecodeBlock(decoded_bits, hamming_block, decode_tools);

        AddDecodedBitsToSizeT(decoded_bits, object_len, decode_tools.remainder_in_bytes, insertable_idx, decode_tools.is_it_end);

        UpdateTools(hamming_block + Hamming::CalcControlBits(hamming_block) + 1, decode_tools);
    }
    ++decode_tools.i;
}

void DecodeAndEncode::DecodeBytesAndPutToFile(char info, size_t hamming_block, std::ofstream& file, IterationTools& decode_tools) {
    decode_tools.buffer_bytes.push_back(info);
    ++decode_tools.current_symbols_count;

    if (decode_tools.current_symbols_count == decode_tools.bytes_count) {
        std::vector<bool> decoded_bits;
        DecodeBlock(decoded_bits, hamming_block, decode_tools);

        AddDecodedBitsToFile(decoded_bits, file, decode_tools.remainder_in_bytes, decode_tools.is_it_end);

        UpdateTools(hamming_block + Hamming::CalcControlBits(hamming_block) + 1, decode_tools);
    }
    ++decode_tools.i;
}

void DecodeAndEncode::EncodeBytesAndPutToArchive(char info, size_t hamming_block, std::ofstream& archive, IterationTools& encode_tools) {
    encode_tools.buffer_bytes.push_back(info);
    ++encode_tools.current_symbols_count;

    if (encode_tools.current_symbols_count == encode_tools.bytes_count) {
        std::vector<bool> encoded_bits;
        EncodeBlock(encoded_bits, hamming_block, encode_tools);

        AddEncodedBitsToArchive(encoded_bits, archive, encode_tools.remainder_in_bytes, encode_tools.is_it_end);

        UpdateTools(hamming_block, encode_tools);
    }

    ++encode_tools.i;
}