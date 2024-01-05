#include "HammingCode.h"
#include <iostream>

const int kBitsCountInByte = 8;

// Вспомогательные функции

size_t Hamming::CalcControlBits(size_t hamming_block) {
    size_t m = 0;
    while ((1 << m) < m + hamming_block + 1) {
        ++m;
    }
    return m;
}

bool ChangeControlBit(const std::vector<bool>& bits, size_t j, size_t degree, size_t len) {
    bool new_control_bit;
    bool first_iter = true;
    for (size_t k = j + 1; k < len - 1; ++k) { // j + 1, т.к смотрим все числа после этой степени двойки
        if ((((k + 1) >> degree) & 1) != 1) {
            continue;
        }
        if (first_iter) {
            new_control_bit = bits[k];
            first_iter = false;
            continue;
        }
        new_control_bit ^= bits[k];
    }
    return new_control_bit;
}

bool ChangeExtraBit(const std::vector<bool>& bits, size_t len) {
    bool new_extra_bit = bits[0];
    for (size_t j = 1; j < len - 1; ++j) {
        new_extra_bit ^= bits[j];
    }
    return new_extra_bit;
}

void SetDefaultBits(std::vector<bool>& bits, char info) {
    for (uint8_t i = 0; i < kBitsCountInByte; ++i) {
        bits.push_back((info >> (kBitsCountInByte - i - 1)) & 1);
    }
}

void AddControlAndExtraBits(const std::vector<bool>& default_bits, std::vector<bool>& coded_bits, size_t len, size_t& current_bool) {
    size_t power_of_two = 1;
    for (size_t j = 0; j < len - 1; ++j) {
        if (j + 1 == power_of_two) {
            coded_bits.push_back(0); // Изначально значение контрольного бита равно 0
            power_of_two <<= 1;
        } else {
            coded_bits.push_back(default_bits[current_bool++]);
        }
    }
    coded_bits.push_back(0); // Изначально значение последнего, дополнительного бита равно 0
}

void SetControlAndExtraBits(std::vector<bool>& coded_bits, size_t hamming_block, size_t control_bits_count, size_t len) {
    size_t power_of_two = 1;
    size_t degree = 0;
    for (size_t j = 0; j < len - 1; ++j) {
        if (j + 1 != power_of_two) {
            continue;
        }
        coded_bits[j] = ChangeControlBit(coded_bits, j, degree, len); // Устанавливаем контрольные биты
        power_of_two <<= 1;
        ++degree;
    }
    coded_bits[len - 1] = ChangeExtraBit(coded_bits, len); // Устаналиваем дополнительный бит
}

void AddToResultingVector(std::vector<uint8_t>& data, const std::vector<bool> new_bits, size_t start, size_t len) {
    uint8_t new_symbol = 0;
    for (size_t j = 0; j < len; ++j) {
        new_symbol |= (new_bits[start + j] << (len - j - 1));
    }
    data.push_back(new_symbol);
}

void AddToDecodedBits(std::vector<bool>& decoded_bits, const std::vector<bool>& coded_bits, size_t len) {
    size_t power_of_two = 1;
    size_t degree = 0;
    for (size_t j = 0; j < len - 1; ++j) {
        if (j + 1 != power_of_two) {
            decoded_bits.push_back(coded_bits[j]);
        } else {
            power_of_two <<= 1;
            ++degree;
        }
    }
}

void DecodeThisSegment(std::vector<bool>& decoded_bits, std::vector<bool>& coded_bits, size_t len) {
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
            bool new_control_bit = ChangeControlBit(coded_bits, j, degree, len);
            if (new_control_bit != coded_bits[j]) {
                ++count_of_errors_while_decoding;
                ind_error_bit |= (1 << degree);
            }
            power_of_two <<= 1;
            ++degree;
        }
    }
    check_all_bits = ChangeExtraBit(coded_bits, len);

    // Если ошибка точно присутствует и дополнительный бит, равный XOR всех битов, не изменился, значит, ошибок 2k, где k >= 1
    if (count_of_errors_while_decoding > 0 && check_all_bits == coded_bits[len - 1]) {
        std::cerr << "Decoding is not possible with Hamming coding, behavior undefined" << '\n';
    }

    // Если ошибка точно присутствует и дополнительный бит изменился, значит, ошибок 2k + 1, где k >= 0.
    // ... ind_error_bit - 1, т.к ind_error_bit - это номер в индексации с 1
    if (count_of_errors_while_decoding > 0 && check_all_bits != coded_bits[len - 1]) {
        coded_bits[ind_error_bit - 1] = !coded_bits[ind_error_bit - 1];
    }

    // Запишем все биты, кроме контрольных и дополнительного, в декодированный вектор
    AddToDecodedBits(decoded_bits, coded_bits, len);
}

// Функции кодирования и декодирования

void Hamming::HammingCode(const std::vector<bool>& default_bits, std::vector<bool>& coded_bits, size_t hamming_block) {

    size_t control_bits_count = Hamming::CalcControlBits(hamming_block) + 1; //Добавим в блок еще один бит, стоящий в конце и отвечающий за четность единичных битов

    size_t current_bool = 0;

    // Добавим контрольные биты в вектор закодированных битов битов

    AddControlAndExtraBits(default_bits, coded_bits, hamming_block + control_bits_count, current_bool);

    // Установим в контрольные биты необходимые значения, а также посчитаем дополнительный контрольный бит в конце

    SetControlAndExtraBits(coded_bits, hamming_block, control_bits_count, hamming_block + control_bits_count);
}

// В декодировании помимо контрольных битов добавим в конец один бит, равный XOR всех битов в его сегменте. Он необходим для обнаружения 2 ошибок (3 и более
// также можно распознавать, но в большинстве реализациях распознается 2 ошибки и исправляется одна. Будем считать, что этот бит является частью контрольных битов,
// поэтому при подсчете контрольных битов сделаем + 1

void Hamming::HammingDecode(std::vector<bool>& coded_bits, std::vector<bool>& decoded_bits, size_t hamming_block) {

    size_t control_bits_count = CalcControlBits(hamming_block) + 1;

    // Декодируем биты

    DecodeThisSegment(decoded_bits, coded_bits, hamming_block + control_bits_count);
}