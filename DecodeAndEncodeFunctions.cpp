#include "DecodeAndEncodeFunctions.h"
#include "HammingCode.h"
#include <fstream>

const uint8_t kBitsInByte = 8;

// Функции для декодирования архива

void DecodeAndEncodeFunctions::ConvertBytesToBits(const std::vector<uint8_t>& bytes, std::vector<bool>& bits, std::vector<bool>& remainder_in_bits_when_take_block, size_t block_in_bits, bool is_remainder, bool is_it_end) {
    for (size_t i = 0; i < bytes.size(); ++i) {
        if (i == bytes.size() - 1 && is_remainder) {
            for (uint8_t j = 0; j < block_in_bits % kBitsInByte; ++j) {
                bits.push_back((bytes.back() >> (kBitsInByte - 1 - j)) & 1);
            }
            if (is_it_end) {
                continue; // В случае декодирования, поскольку при паддинге мы добавляли нули в конец, их не учитываем, т.е биты в остатке не берем
            }
            for (uint8_t j = block_in_bits % kBitsInByte; j < kBitsInByte; ++j) {
                remainder_in_bits_when_take_block.push_back((bytes.back() >> (kBitsInByte - 1 - j)) & 1);
            }
        } else {
            for (uint8_t j = 0; j < kBitsInByte; ++j) {
                bits.push_back((bytes[i] >> (kBitsInByte - 1 - j)) & 1);
            }
        }
    }
}

void DecodeAndEncodeFunctions::ConvertDecodedBitsToDecodedBytes(std::vector<bool>& decoded_bits, std::vector<uint8_t>& decoded_bytes, std::vector<bool>& remainder) {
    for (size_t i = 0; i < decoded_bits.size(); ++i) {
        remainder.push_back(decoded_bits[i]);
    }

    for (size_t i = 0; i < (remainder.size() / kBitsInByte); ++i) {
        uint8_t info = 0;
        for (uint8_t j = 0; j < kBitsInByte; ++j) {
            info |= (remainder[i * kBitsInByte + j] << (kBitsInByte - 1 - j));
        }
        decoded_bytes.push_back(info);
    }

    if (remainder.size() % 8 != 0) {
        uint8_t how_much_is_left = remainder.size() % kBitsInByte;
        uint8_t new_remainder[kBitsInByte];

        for (uint8_t i = 0; i < how_much_is_left; ++i) {
            new_remainder[i] = remainder[(remainder.size() / kBitsInByte) * kBitsInByte + i];
        }
        remainder.clear();

        for (uint8_t i = 0; i < how_much_is_left; ++i) {
            remainder.push_back(new_remainder[i]);
        }
    } else {
        remainder.clear();
    }
}

void DecodeAndEncodeFunctions::IterationOverCodedSymbols(char info, size_t hamming_block, std::vector<uint8_t>& decoded_bytes, IterationTools& decode_tools) {
    decode_tools.buffer_bytes.push_back(info);
    ++decode_tools.current_symbols_count;

    if (decode_tools.current_symbols_count == decode_tools.bytes_count) {

        ConvertBytesToBits(decode_tools.buffer_bytes, decode_tools.buffer_bits, decode_tools.remainder_in_bits_when_take_block, decode_tools.bits_count, (decode_tools.bytes_count * kBitsInByte - decode_tools.bits_count != 0), decode_tools.is_it_end);

        std::vector<bool> decoded_bits;
        Hamming::HammingDecode(decode_tools.buffer_bits, decoded_bits, hamming_block);

        ConvertDecodedBitsToDecodedBytes(decoded_bits, decoded_bytes, decode_tools.remainder_in_bits_when_convert_to_bytes);

        if (decode_tools.bytes_count * kBitsInByte - decode_tools.bits_count != 0) {
            decode_tools.buffer_bits = decode_tools.remainder_in_bits_when_take_block;

            decode_tools.remainder = decode_tools.buffer_bits.size();

            decode_tools.bits_count = hamming_block + Hamming::CalcControlBits(hamming_block) + 1 - decode_tools.remainder;
        } else {
            decode_tools.buffer_bits.clear();

            decode_tools.remainder = 0;

            decode_tools.bits_count = hamming_block + Hamming::CalcControlBits(hamming_block) + 1;
        }

        decode_tools.bytes_count = decode_tools.bits_count / kBitsInByte + (decode_tools.bits_count % kBitsInByte != 0);
        decode_tools.buffer_bytes.clear();
        decode_tools.remainder_in_bits_when_take_block.clear();
        decode_tools.current_symbols_count = 0;
    }

    ++decode_tools.i;
}

// Функции для кодирования архива

void DecodeAndEncodeFunctions::ConvertDefaultBytesToDefaultBits(const std::vector<uint8_t>& bytes, std::vector<bool>& bits, std::vector<bool>& remainder_in_bits_when_take_block, size_t block_in_bits, bool is_remainder) {
    for (size_t i = 0; i < bytes.size(); ++i) {
        if (i == bytes.size() - 1 && is_remainder) {
            for (uint8_t j = 0; j < block_in_bits % kBitsInByte; ++j) {
                bits.push_back((bytes.back() >> (kBitsInByte - 1 - j)) & 1);
            }
            for (uint8_t j = block_in_bits % kBitsInByte; j < kBitsInByte; ++j) {
                remainder_in_bits_when_take_block.push_back((bytes.back() >> (kBitsInByte - 1 - j)) & 1);
            }
        } else {
            for (uint8_t j = 0; j < kBitsInByte; ++j) {
                bits.push_back((bytes[i] >> (kBitsInByte - 1 - j)) & 1);
            }
        }
    }
}

void DecodeAndEncodeFunctions::ConvertCodedBitsToCodedBytes(const std::vector<bool>& coded_bits, std::vector<uint8_t>& coded_bytes, std::vector<bool>& remainder, bool is_it_end_of_the_file) {
    for (size_t i = 0; i < coded_bits.size(); ++i) {
        remainder.push_back(coded_bits[i]);
    }

    for (size_t i = 0; i < (remainder.size() / kBitsInByte); ++i) {
        uint8_t info = 0;
        for (uint8_t j = 0; j < kBitsInByte; ++j) {
            info |= (remainder[i * kBitsInByte + j] << (kBitsInByte - 1 - j));
        }
        coded_bytes.push_back(info);
    }

    if (remainder.size() % kBitsInByte != 0) {
        uint8_t how_much_is_left = remainder.size() % kBitsInByte;
        uint8_t new_remainder[kBitsInByte];

        for (uint8_t i = 0; i < how_much_is_left; ++i) {
            new_remainder[i] = remainder[(remainder.size() / kBitsInByte) * kBitsInByte + i];
        }
        remainder.clear();

        for (uint8_t i = 0; i < how_much_is_left; ++i) {
            remainder.push_back(new_remainder[i]);
        }

        if (is_it_end_of_the_file) {
            uint8_t remember_this_size = remainder.size();
            for (size_t j = 0; j < kBitsInByte - remember_this_size; ++j) {
                remainder.push_back(0); // Добавляем в конец нули в качестве паддинга до размера 8, чтобы поместить в байт (так удобнее декодировать)
            }

            uint8_t info = 0;
            for (uint8_t j = 0; j < kBitsInByte; ++j) {
                info |= (remainder[j] << (kBitsInByte - 1 - j));
            }
            coded_bytes.push_back(info);
        }
    } else {
        remainder.clear();
    }
}

void DecodeAndEncodeFunctions::IteratingOverDefaultSymbols(char info, size_t hamming_block, std::fstream& archive, IterationTools& encode_tools) {
    encode_tools.buffer_bytes.push_back(info);
    ++encode_tools.current_symbols_count;

    if (encode_tools.current_symbols_count == encode_tools.bytes_count) {
        ConvertDefaultBytesToDefaultBits(encode_tools.buffer_bytes, encode_tools.buffer_bits, encode_tools.remainder_in_bits_when_take_block, encode_tools.bits_count, (encode_tools.bytes_count * kBitsInByte - encode_tools.bits_count != 0));

        std::vector<bool> coded_bits;
        Hamming::HammingCode(encode_tools.buffer_bits, coded_bits, hamming_block);

        std::vector<uint8_t> coded_bytes;
        ConvertCodedBitsToCodedBytes(coded_bits, coded_bytes, encode_tools.remainder_in_bits_when_convert_to_bytes, encode_tools.is_it_end);

        for (size_t j = 0; j < coded_bytes.size(); ++j) {
            archive << coded_bytes[j];
        }

        if (encode_tools.bytes_count * kBitsInByte - encode_tools.bits_count != 0) {
            encode_tools.buffer_bits = encode_tools.remainder_in_bits_when_take_block;

            encode_tools.remainder = encode_tools.buffer_bits.size();

            encode_tools.bits_count = hamming_block - encode_tools.remainder;
        } else {
            encode_tools.buffer_bits.clear();

            encode_tools.remainder = 0;

            encode_tools.bits_count = hamming_block;
        }

        encode_tools.bytes_count = encode_tools.bits_count / kBitsInByte + (encode_tools.bits_count % kBitsInByte != 0);
        encode_tools.buffer_bytes.clear();
        encode_tools.remainder_in_bits_when_take_block.clear();
        encode_tools.current_symbols_count = 0;
    }

    ++encode_tools.i;
}