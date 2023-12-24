#include "HammingCode.h"

#include <fstream>
#include <iostream>

const int kBitsInByte = 8;
const int kSizeOfEachNameInBytes = 15;

bool ChangeControlBit(std::vector<bool>& bits, size_t i, size_t j, size_t degree, size_t block_for_coding, size_t count_of_control_bits) {
    bool result_for_this_control_bit;
    bool first_iter = 1;
    for (int k = j; k < block_for_coding + count_of_control_bits; ++k) {
        if ((((k + 1) >> degree) & 1) == 1) {
            if (first_iter) {
                result_for_this_control_bit = bits[i * (block_for_coding + count_of_control_bits) + k];
                first_iter = 0;
                continue;
            }
            result_for_this_control_bit ^= bits[i * (block_for_coding + count_of_control_bits) + k];
        }
    }
    return result_for_this_control_bit;
}

size_t Hamming::CalcControlBits(size_t hamming_block) {
    size_t m = 0;
    while ((1 << m) < m + hamming_block + 1) {
        ++m;
    }
    return m;
}

size_t Hamming::HammingCode(std::vector<uint8_t>& data, size_t hamming_block, const std::string& filename, bool create_file) {
    std::vector<bool> bits;

    if (create_file) {
        std::fstream file;
        file.open(filename, std::ios::in);
        char info;
        while (file.get(info)) {
            for (int i = kBitsInByte - 1; i >= 0; --i) {
                bits.push_back(info >> i);
            }
        }
    } else {
        size_t ptr = 0;
        while (filename[ptr] != '\0') {
            for (int i = kBitsInByte - 1; i >= 0; --i) {
                bits.push_back(filename[ptr] >> i);
            }
        }
    }

    size_t count_of_operations = bits.size() / hamming_block;
    size_t count_of_control_bits = Hamming::CalcControlBits(hamming_block);

    std::vector<bool> new_bits;
    size_t current_bool = 0;

    size_t power_of_two = 1;

    for (size_t i = 0; i < count_of_operations; ++i) {
        power_of_two = 1;
        for (size_t j = 1; j <= hamming_block + count_of_control_bits; ++j) {
            if (j == power_of_two) {
                new_bits.push_back(0);
                power_of_two <<= 1;
            } else {
                new_bits.push_back(bits[current_bool++]);
            }
        }
    }

    //Если кол-во битов нацело не делится на hamming_block, не забудем добавить контрольные биты также и в остаток

    size_t count_of_control_bits_for_remainder = Hamming::CalcControlBits(bits.size() % hamming_block);

    power_of_two = 1;

    for (size_t i = 1; i <= bits.size() % hamming_block + count_of_control_bits_for_remainder; ++i) {
        if (i == power_of_two) {
            new_bits.push_back(0);
            power_of_two <<= 1;
        } else {
            new_bits.push_back(bits[current_bool++]);
        }
    }

    // Установим в контрольные биты необходимые значения
    // Тут необходимо правильно итерироваться по вектору new_bits, добавляем к j число i * (count_of_control_bits + hamming_block) - столько битов
    // уже было рассмотрено

    for (size_t i = 0; i < count_of_operations; ++i) {
        power_of_two = 1;
        size_t degree = 0;
        for (size_t j = 0; j < hamming_block + count_of_control_bits; ++j) {
            if (j + 1 != power_of_two) {
                continue;
            }
            new_bits[i * (hamming_block + count_of_control_bits) + j] = ChangeControlBit(new_bits, i, j, degree, hamming_block, count_of_control_bits);
            power_of_two <<= 1;
            ++degree;
        }
    }

    //Не забудем сделать операцию XOR для остатка

    for (size_t i = 0; i < new_bits.size() % (hamming_block + count_of_control_bits); ++i) {
        power_of_two = 1;
        size_t degree = 0;
        if (i + 1 != power_of_two) {
            continue;
        }
        new_bits[count_of_operations * (hamming_block + count_of_control_bits) + i] = ChangeControlBit(new_bits, count_of_operations, i, degree, bits.size() % hamming_block
        , count_of_control_bits_for_remainder);
        power_of_two <<= 1;
        ++degree;
    }

    //Мы закодировали битовую последовательность алгоритмом Хемминга, теперь добавим ее в вектор data;

    //Кроме того, добавим ведущие нули в том случае, если при кодировании имени файла его длина составляет меньше 15 байт. Если есть остаток, то добавляем
    //на 1 меньше, за это отвечает is_remainder

    if (!create_file) {
        bool is_remainder = new_bits.size() % 8;
        for (size_t i = 0; i < kSizeOfEachNameInBytes - (new_bits.size() / 8) - is_remainder; ++i) {
            data.push_back(0);
        }
    }

    for (size_t i = 0; i < new_bits.size() / 8; ++i) {
        uint8_t new_symbol = 0;
        for (size_t j = 0; j < kBitsInByte; ++j) {
            new_symbol |= (new_bits[i * 8 + j] << 7 - j);
        }
        data.push_back(new_symbol);
    }

    uint8_t new_symbol = 0;
    for (size_t i = (new_bits.size() / 8) * 8; i < new_bits.size(); ++i) {
        new_symbol |= (new_bits[i] << (new_bits.size() - (new_bits.size() / 8) * 8) - 1);
        data.push_back(new_symbol);
    }

    bool is_remainder = (new_bits.size() / 8) % 8;
    return new_bits.size() / 8 + is_remainder;
}

std::vector<uint8_t> Hamming::HammingDecode(std::vector<uint8_t>& data, size_t hamming_block) {
    std::vector<bool> bits;
    for (size_t i = 0; i < data.size(); ++i) {
        for (size_t j = kBitsInByte - 1; j >= 0; --j) {
            bits.push_back(data[i] >> j);
        }
    }

    size_t control_bits = CalcControlBits(hamming_block);
    size_t count_of_operations = bits.size() / (hamming_block + control_bits);

    std::vector<bool> new_bits;
    for (size_t i = 0; i < count_of_operations; ++i) {
        size_t power_of_two = 1;
        size_t degree = 0;
        size_t count_of_errors_while_decoding = 0;

        //Создадим переменную, являющуюся номером искомого бита, где находится ошибка. Если при пересчете в контрольном бите 2^k будет несовпадение, значит,
        //номер искомого бита имеет 1 на k-ом месте
        size_t ind_error_bit = 0;

        //Для каждого бита с номером 2^k (нумерация относительно начала блока) посчитаем в new_bit XOR всех битов блока, номер которых содержит 1 на k-ом бите
        for (size_t j = 0; j < hamming_block + control_bits; ++j) {
            if (j + 1 == power_of_two) {
                bool new_bit = ChangeControlBit(bits, i, j, degree, hamming_block, control_bits);
                if (new_bit != bits[i * (hamming_block + control_bits) + j]) {
                    ++count_of_errors_while_decoding;
                    ind_error_bit |= (1 << j);
                }
                power_of_two <<= 1;
                ++degree;
            }
        }

        //Инвертируем бит, если есть ошибка. Заметим также, что есть случай, когда "испортившийся бит" - сам контрольный бит, в таком случае все информационные
        //биты сохранены, так как изменение ниже затронет только контрольный бит
        if (ind_error_bit > 0) {
            bits[i * (hamming_block + control_bits) + ind_error_bit] = !bits[i * (hamming_block + control_bits) + ind_error_bit];
        }
        //Если хоть в каком-то блоке кол-во ошибок в контрольных битах > 1, декодировать исходную битовую последовательность нельзя, т.к нельзя декодировать блок
        if (count_of_errors_while_decoding > 1) {
            std::cerr << "Decoding is not possible with Hamming coding";
        }

        power_of_two = 1;
        degree = 0;
        for (size_t j = 0; j < hamming_block + control_bits; ++j) {
            if (j + 1 != power_of_two) {
                new_bits.push_back(bits[i * (hamming_block + control_bits) + j]);
            } else {
                power_of_two <<= 1;
                ++degree;
            }
        }
    }

    //Не забудем декодировать остаток
    for (size_t i = 0; i < bits.size() % (hamming_block + control_bits); ++i) {
        size_t power_of_two = 1;
        size_t degree = 0;
        size_t count_of_error_while_decoding = 0;

        size_t ind_error_bit = 0;

        size_t control_bits_for_remainder = CalcControlBits(bits.size() % (hamming_block + control_bits));

        for (size_t j = 0; j < bits.size() % (hamming_block + control_bits); ++j) {
            if (j + 1 == power_of_two) {
                bool new_bit = ChangeControlBit(bits, count_of_operations, j, degree, bits.size() % (hamming_block + control_bits) - control_bits_for_remainder, control_bits_for_remainder);
                if (new_bit != bits[count_of_operations * (hamming_block + control_bits) + j]) {
                    ++count_of_error_while_decoding;
                    ind_error_bit |= (1 << j);
                }
                power_of_two <<= 1;
                ++degree;
            }
        }

        if (ind_error_bit > 0) {
            bits[count_of_operations * (hamming_block + control_bits) + ind_error_bit] = !bits[count_of_operations * (hamming_block + control_bits) + ind_error_bit];
        }

        if (count_of_error_while_decoding > 1) {
            std::cerr << "Decoding is not possible with Hamming coding";
        }

        power_of_two = 1;
        degree = 0;
        for (size_t j = 0; j < bits.size() % (hamming_block + control_bits); ++j) {
            if (j + 1 != power_of_two) {
                new_bits.push_back(bits[count_of_operations * (hamming_block + control_bits) + j]);
            } else {
                power_of_two <<= 1;
                ++degree;
            }
        }
    }

    //Преобразуем булевый вектор в вектор чаров;
    std::vector<uint8_t> decode_string;

    for (size_t i = 0; i < new_bits.size() / 8; ++i) {
        uint8_t new_char = 0;
        for (size_t j = 0; j < kBitsInByte; ++j) {
            new_char |= (new_bits[i * 8 + j] << (kBitsInByte - 1 - j));
        }
        decode_string.push_back(new_char);
    }

    uint8_t new_char = 0;
    for (size_t i = 0; i < new_bits.size() % 8; ++i) {
        new_char |= (new_bits[new_bits.size() / 8 * 8 + i] << (new_bits.size() % 8 - 1 - i));
    }
    decode_string.push_back(new_char);

    return decode_string;
}