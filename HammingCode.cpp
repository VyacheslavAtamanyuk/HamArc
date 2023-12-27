#include "HammingCode.h"

#include <fstream>
#include <iostream>

const int kBitsCountInByte = 8;
const int kSizeOfEachNameInBytes = 15;

// Вспомогательные функции

bool ChangeControlBit(std::vector<bool>& bits, size_t i, size_t j, size_t degree, size_t block_for_coding, size_t control_bits_count, size_t len) {
    bool new_control_bit;
    bool first_iter = true;
    for (size_t k = j; k < len - 1; ++k) {
        if ((((k + 1) >> degree) & 1) == 1) {
            if (first_iter) {
                new_control_bit = bits[i * (block_for_coding + control_bits_count) + k];
                first_iter = false;
                continue;
            }
            new_control_bit ^= bits[i * (block_for_coding + control_bits_count) + k];
        }
    }
    return new_control_bit;
}

bool ChangeExtraBit(std::vector<bool>& bits, size_t i, size_t block_for_coding, size_t control_bits_count, size_t len) {
    bool new_extra_bit;
    for (size_t j = 0; j < len - 1; ++j) {
        if (j == 0) {
            new_extra_bit = bits[i * (block_for_coding + control_bits_count)];
            continue;
        }
        new_extra_bit ^= bits[i * (block_for_coding + control_bits_count) + j];
    }
    return new_extra_bit;
}

size_t Hamming::CalcControlBits(size_t hamming_block) {
    size_t m = 0;
    while ((1 << m) < m + hamming_block + 1) {
        ++m;
    }
    return m;
}

void AddNewBits(std::vector<bool>& old_bits, std::vector<bool>& new_bits, size_t len, size_t& current_bool) {
    size_t power_of_two = 1;
    for (size_t j = 0; j < len - 1; ++j) {
        if (j + 1 == power_of_two) {
            new_bits.push_back(0); // Изначально значение контрольного бита равно 0
            power_of_two <<= 1;
        } else {
            new_bits.push_back(old_bits[current_bool++]);
        }
    }
    new_bits.push_back(0); // Изначально значение последнего бита равно 0
}

void SetControlAndExtraBits(std::vector<bool>& new_bits, size_t hamming_block, size_t control_bits_count, size_t len, size_t i) {
    size_t power_of_two = 1;
    size_t degree = 0;
    for (size_t j = 0; j < len - 1; ++j) {
        if (j + 1 != power_of_two) {
            continue;
        }
        new_bits[i * (hamming_block + control_bits_count) + j] = ChangeControlBit(new_bits, i, j, degree, hamming_block, control_bits_count, len); // Устанавливаем контрольные биты
        power_of_two <<= 1;
        ++degree;
    }
    new_bits[i * (hamming_block + control_bits_count) + len - 1] = ChangeExtraBit(new_bits, i, hamming_block, control_bits_count, len); // Устаналиваем дополнительный бит
}

void PadToFiveteenBits(std::vector<uint8_t>& data, std::vector<bool>& new_bits, bool create_file) {
    if (!create_file) {
        bool is_remainder = new_bits.size() % 8;
        for (size_t i = 0; i < kSizeOfEachNameInBytes - (new_bits.size() / 8) - is_remainder; ++i) {
            data.push_back(0);
        }
    }
}

void AddToResultingVector(std::vector<uint8_t>& data, std::vector<bool> new_bits, size_t start, size_t len) {
    uint8_t new_symbol = 0;
    for (size_t j = 0; j < len; ++j) {
        new_symbol |= (new_bits[start + j] << (len - j - 1));
    }
    data.push_back(new_symbol);
}

void AddToDecodedBits(std::vector<bool>& decoded_bits, std::vector<bool>& old_bits, size_t hamming_block, size_t control_bits_count, size_t i, size_t len) {
    size_t power_of_two = 1;
    size_t degree = 0;
    for (size_t j = 0; j < len - 1; ++j) {
        if (j + 1 != power_of_two) {
            decoded_bits.push_back(old_bits[i * (hamming_block + control_bits_count) + j]);
        } else {
            power_of_two <<= 1;
            ++degree;
        }
    }
}

void DecodeThisSegment(std::vector<bool>& decoded_bits, std::vector<bool>& old_bits, size_t hamming_block, size_t control_bits_count, size_t i, size_t len) {
    size_t power_of_two = 1;
    size_t degree = 0;
    size_t count_of_errors_while_decoding = 0;

    //Создадим переменную ind_error_bit, являющуюся номером искомого бита, где находится ошибка. Если при пересчете в контрольном бите 2^k будет несовпадение, значит,
    //номер искомого бита имеет 1 на k-ом месте. check_all_bits - дополнительный бит в конце
    size_t ind_error_bit = 0;
    bool check_all_bits;

    // Для каждого бита с номером 2^k (нумерация относительно начала сегмента) посчитаем в new_control_bit XOR всех битов блока, номер которых содержит 1 на k-ом бите
    // Поскольку последний бит сегмента по сути не относится ни к контрольным, ни к информационным битам, не будем его учитывать, поэтому - 1
    for (size_t j = 0; j < len - 1; ++j) {
        if (j + 1 == power_of_two) {
            bool new_control_bit = ChangeControlBit(old_bits, i, j, degree, hamming_block, control_bits_count, len);
            if (new_control_bit != old_bits[i * (hamming_block + control_bits_count) + j]) {
                ++count_of_errors_while_decoding;
                ind_error_bit |= (1 << j);
            }
            power_of_two <<= 1;
            ++degree;
        }
    }
    check_all_bits = ChangeExtraBit(old_bits, i, hamming_block, control_bits_count, len);

    // Если ошибка точно присутствует и дополнительный бит, равный XOR всех битов, не изменился, значит, ошибок 2k, где k >= 1
    // ... len - 1, т.к последний бит сегмента не рассматриваем
    if (count_of_errors_while_decoding > 0 && check_all_bits == old_bits[i * (hamming_block + control_bits_count) + len - 1]) {
        std::cerr << "Decoding is not possible with Hamming coding, behavior undefined" << '\n';
    }

    // Если ошибка точно присутствует и дополнительный бит изменился, значит, ошибок 2k + 1, где k >= 0.
    // ... ind_error_bit - 1, т.к ind_error_bit - это номер в индексации с 1
    if (count_of_errors_while_decoding > 0 && check_all_bits != old_bits[i * (hamming_block + control_bits_count) + len - 1]) {
        old_bits[i * (hamming_block + control_bits_count) + ind_error_bit - 1] = !old_bits[i * (hamming_block + control_bits_count) + ind_error_bit - 1];
    }

    // Запишем все биты, кроме контрольных и дополнительного, в декодированный вектор
    AddToDecodedBits(decoded_bits, old_bits, hamming_block, control_bits_count, i, len);
}

// Кодирование и декодирование

size_t Hamming::HammingCode(std::vector<uint8_t>& data, size_t hamming_block, const std::string& filename, bool create_file) {
    std::vector<bool> bits;

    if (create_file) {
        std::fstream file;
        file.open(filename, std::ios::in);
        char info;
        while (file.get(info)) {
            for (int i = kBitsCountInByte - 1; i >= 0; --i) {
                bits.push_back(info >> i);
            }
        }
    } else {
        size_t ptr = 0;
        while (filename[ptr] != '\0') {
            for (int i = kBitsCountInByte - 1; i >= 0; --i) {
                bits.push_back(filename[ptr] >> i);
            }
        }
    }

    size_t operations_count = bits.size() / hamming_block;
    size_t control_bits_count = Hamming::CalcControlBits(hamming_block) + 1; //Добавим в каждый блок еще один бит, стоящий в конце и отвечающий за четность единичных битов

    std::vector<bool> new_bits;
    size_t current_bool = 0;

    // Добавим контрольные биты в вектор новых битов

    for (size_t i = 0; i < operations_count; ++i) {
        AddNewBits(bits, new_bits, hamming_block + control_bits_count, current_bool);
    }

    //Если кол-во битов нацело не делится на hamming_block, добавим контрольные биты в остаток
    if (bits.size() % hamming_block != 0) {
        size_t control_bits_count_for_remainder = Hamming::CalcControlBits(bits.size() % hamming_block) + 1;

        AddNewBits(bits, new_bits, bits.size() % hamming_block + control_bits_count_for_remainder, current_bool);
    }

    // Установим в контрольные биты необходимые значения, а также посчитаем дополнительный контрольный бит в конце

    for (size_t i = 0; i < operations_count; ++i) {
        SetControlAndExtraBits(new_bits, hamming_block, control_bits_count, hamming_block + control_bits_count, i);
    }

    // Установим в контрольные биты остатка необходимые значения, а также посчитаем дополнительный контрольный бит в конце
    if (bits.size() % hamming_block != 0) {
        SetControlAndExtraBits(new_bits, hamming_block, control_bits_count, new_bits.size() % (hamming_block + control_bits_count), operations_count);
    }

    // Добавим ведущие нули в том случае, если кодируется имя файла и его длина составляет меньше 15 символов. Если есть остаток, то добавляем
    // на 1 меньше, за это отвечает is_remainder в функции PadToFiveteenBits

    PadToFiveteenBits(data, new_bits, create_file);

    // Если кодируется не имя файла, а сам файл, то заполним итоговый вектор чаров байтами

    for (size_t i = 0; i < new_bits.size() / kBitsCountInByte; ++i) {
        AddToResultingVector(data, new_bits, i * kBitsCountInByte, kBitsCountInByte);
    }

    AddToResultingVector(data, new_bits, (new_bits.size() / kBitsCountInByte) * kBitsCountInByte, new_bits.size() % kBitsCountInByte);

    bool is_remainder = new_bits.size() % kBitsCountInByte;
    return new_bits.size() / kBitsCountInByte + is_remainder;
}

// В декодировании помимо контрольных битов добавим в конец один бит, равный XOR всех битов в его сегменте. Он необходим для обнаружения 2 ошибок (3 и более
// также можно распознавать, но в большинстве реализациях распознается 2 ошибки и исправляется одна. Будем считать, что этот бит является частью контрольных битов,
// поэтому при подсчете контрольных битов сделаем + 1

std::vector<uint8_t> Hamming::HammingDecode(std::vector<uint8_t>& data, size_t hamming_block) {
    std::vector<bool> old_bits;
    for (size_t i = 0; i < data.size(); ++i) {
        for (size_t j = kBitsCountInByte - 1; j >= 0; --j) {
            old_bits.push_back(data[i] >> j);
        }
    }

    size_t control_bits_count = CalcControlBits(hamming_block) + 1;
    size_t operations_count = old_bits.size() / (hamming_block + control_bits_count);

    // Декодируем биты
    std::vector<bool> decoded_bits;
    for (size_t i = 0; i < operations_count; ++i) {
        DecodeThisSegment(decoded_bits, old_bits, hamming_block, control_bits_count, i, hamming_block + control_bits_count);
    }

    DecodeThisSegment(decoded_bits, old_bits, hamming_block, control_bits_count, operations_count, old_bits.size() % (hamming_block + control_bits_count));

    //Преобразуем булевый вектор в вектор чаров;
    std::vector<uint8_t> result;
    for (size_t i = 0; i < decoded_bits.size() / kBitsCountInByte; ++i) {
        AddToResultingVector(result, decoded_bits, i * kBitsCountInByte, kBitsCountInByte);
    }

    AddToResultingVector(result, decoded_bits, (decoded_bits.size() / kBitsCountInByte) * kBitsCountInByte, decoded_bits.size() % kBitsCountInByte);

    return result;
}