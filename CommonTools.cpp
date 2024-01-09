#include "CommonTools.h"
#include "DecodeAndEncode.h"
#include "HammingCode.h"
#include <cstring>
#include <filesystem>

const uint8_t kBitsInByte = 8;
const uint8_t kBytesInSizeT = 8;
const uint8_t kCodedLenSizeInBytes = 13;
const uint8_t kFixedHammingBlockToLen = 8;

CommonTools::CommonTools(std::ifstream& archive, const std::string& archive_name, size_t hamming_block): archive(archive), archive_name(archive_name), hamming_block(hamming_block) {}

// Получение длины закодированного файла / имени файла в байтах

size_t CommonTools::GetLen() {
    size_t len_in_bits = 0;

    IterationTools tools_to_iterate(hamming_block + Hamming::CalcControlBits(hamming_block) + 1);
    DecodeAndEncode iterator;

    uint8_t idx = 0;

    while (tools_to_iterate.i < kCodedLenSizeInBytes && std::filesystem::file_size(archive_name) > archive.tellg()) {
        tools_to_iterate.is_it_end = (tools_to_iterate.i == kCodedLenSizeInBytes - 1);
        char info = archive.get();

        iterator.DecodeBytesAndPutToSizeT(info, hamming_block, len_in_bits, tools_to_iterate, idx);
    }

    return (len_in_bits / kBitsInByte) + (len_in_bits % kBitsInByte != 0);
}

// Получение следующего имени файла

void CommonTools::GetNextFilename(std::string& original_filename) {
    size_t len_in_bytes = GetLen();

    IterationTools iterate_over_filename(hamming_block + Hamming::CalcControlBits(hamming_block) + 1);
    DecodeAndEncode iterator;

    while (iterate_over_filename.i < len_in_bytes && std::filesystem::file_size(archive_name) > archive.tellg()) {
        iterate_over_filename.is_it_end = (iterate_over_filename.i == len_in_bytes - 1);
        char info = archive.get();

        iterator.DecodeBytesAndPutToString(info, hamming_block, original_filename, iterate_over_filename);
    }
}

// Получение следующего файла

void CommonTools::GetNextFile(std::ofstream& file) {
    size_t len_in_bytes = GetLen();

    IterationTools iterate_over_file(hamming_block + Hamming::CalcControlBits(hamming_block) + 1);
    DecodeAndEncode iterator;

    while (iterate_over_file.i < len_in_bytes && std::filesystem::file_size(archive_name) > archive.tellg()) {
        iterate_over_file.is_it_end = (iterate_over_file.i == len_in_bytes - 1);
        char info = archive.get();

        iterator.DecodeBytesAndPutToFile(info, hamming_block, file, iterate_over_file);
    }
}

// Считывание следующей длины в битах и пропуск соответствующего количества байтов

void CommonTools::Skip() {
    size_t len_in_bytes = GetLen();

    archive.seekg(len_in_bytes, std::ios_base::cur);
}

// Запись в файл закодированного имена файла и его длины (на момент вызова функции указатель в файле находится в конце закодированного имени файла)

void CommonTools::WriteEncodedLenAndFilenameBytes(size_t original_filename_len, std::ofstream& file) {
    size_t filename_len_in_bits = CalcCodedFilenameOrFileLenInBits(hamming_block, original_filename_len);
    size_t filename_len_in_bytes = filename_len_in_bits / kBitsInByte + (filename_len_in_bits % kBitsInByte != 0);

    archive.seekg(-(filename_len_in_bytes + kCodedLenSizeInBytes), std::ios::cur);

    size_t i = 0;
    while (i < filename_len_in_bytes + kCodedLenSizeInBytes && std::filesystem::file_size(archive_name) > archive.tellg()) {
        char info = archive.get();
        file << info;

        ++i;
    }
}

// Запись в файл закодированного файла и его длины (на момент вызова функции указатель в файле находится в начале длины)

void CommonTools::WriteEncodedLenAndFileBytes(std::ofstream& file) {
    size_t file_len_in_bytes = GetLen();

    archive.seekg(-kCodedLenSizeInBytes, std::ios::cur);

    size_t i = 0;
    while (i < (kCodedLenSizeInBytes + file_len_in_bytes) && std::filesystem::file_size(archive_name) > archive.tellg()) {
        char info = archive.get();
        file << info;

        ++i;
    }
}

// Подсчет длины у закодированного объекта, исходная длина которого default_len

size_t CommonTools::CalcCodedFilenameOrFileLenInBits(size_t hamming_block, size_t default_len) {
    size_t remainder = (kBitsInByte * default_len) % hamming_block;

    size_t result = (kBitsInByte * default_len / hamming_block) * (hamming_block + Hamming::CalcControlBits(hamming_block) + 1);

    if (remainder != 0) {
        result += (remainder + Hamming::CalcControlBits(remainder) + 1);
    }

    return result;
}

// Проверка на то, надо ли извлекать файл

bool CommonTools::IsExtractableFile(const std::vector<const char*>& extractable_files, const std::string& origin_filename) {
    for (size_t i = 0; i < extractable_files.size(); ++i) {
        if (origin_filename == extractable_files[i]) {
            return true;
        }
    }
    return false;
}

// Проверка на то, надо ли удалять файл

bool CommonTools::IsDeletableFile(const std::vector<const char*>& deletable_files, const std::string& origin_filename) {
    for (size_t i = 0; i < deletable_files.size(); ++i) {
        if (origin_filename == deletable_files[i]) {
            return true;
        }
    }
    return false;
}