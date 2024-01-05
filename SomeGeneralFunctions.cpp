#include "SomeGeneralFunctions.h"
#include "DecodeAndEncodeFunctions.h"
#include "HammingCode.h"
#include <filesystem>

const uint8_t kBitsInByte = 8;
const uint8_t kBytesInSizeT = 8;
const uint8_t kCodedLenSizeInBytes = 13;
const uint8_t kFixedHammingBlockToLen = 8;

void SomeGeneralFunctions::GetNextNthDecodedBytes(std::ifstream& archive, const std::string& archive_path, size_t hamming_block, size_t n, std::vector<uint8_t>& coded_bytes, std::vector<uint8_t>& decoded_bytes) {
    IterationTools tools_to_get_len;

    tools_to_get_len.bits_count = hamming_block + Hamming::CalcControlBits(hamming_block) + 1;
    tools_to_get_len.bytes_count = tools_to_get_len.bits_count / kBitsInByte + (tools_to_get_len.bits_count % kBitsInByte != 0);

    while (tools_to_get_len.i < n && std::filesystem::file_size(archive_path) > archive.tellg()) {
        tools_to_get_len.is_it_end = (tools_to_get_len.i == n - 1);
        char info = archive.get();

        coded_bytes.push_back(info);

        DecodeAndEncodeFunctions::IterationOverCodedSymbols(info, hamming_block, decoded_bytes, tools_to_get_len);
    }
}

// Конвертация элементов вектора типа uint8_t в тип size_t (гарантируется, что размер вектора равен kBytesInSizeT)

size_t SomeGeneralFunctions::ConvertBytesToSizeT(const std::vector<uint8_t>& bytes) {
    size_t number = 0;
    for (uint8_t i = 0; i < kBytesInSizeT; ++i) {
        number |= (bytes[i] << (kBitsInByte * (kBitsInByte - 1 - i)));
    }
    return number;
}

// Задать элементы массива чаров через элементы вектора типа uint8_t и установить нуль-терминатор в конец

void SomeGeneralFunctions::GetPointerOnCharFromDecodedObject(const std::vector<uint8_t>& object, char* ptr) {
    for (size_t i = 0; i < object.size(); ++i) {
        ptr[i] = object[i];
    }
    ptr[object.size()] = '\0';
}

// Вычисляет длину (в битах) у следующего объекта (файл / имя файла) в битах, переводит данное число в байты и пропускает такое количество байт

void SomeGeneralFunctions::Skip(std::ifstream& archive, const std::string& archive_path, size_t hamming_block) {
    std::vector<uint8_t> coded_len_bytes, decoded_len_bytes;

    GetNextNthDecodedBytes(archive, archive_path, hamming_block, kCodedLenSizeInBytes, coded_len_bytes, decoded_len_bytes);

    size_t len_in_bits = ConvertBytesToSizeT(decoded_len_bytes);
    size_t len_in_bytes = len_in_bits / kBitsInByte + (len_in_bits % kBitsInByte != 0);

    archive.seekg(len_in_bytes, std::ios_base::cur);
}