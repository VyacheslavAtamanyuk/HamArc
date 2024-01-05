#include "Archive.h"
#include "CreateFunctions.h"
#include "DecodeAndEncodeFunctions.h"

const uint8_t kBytesInSizeT = 8;
const uint8_t kBitsInByte = 8;
const uint8_t kFixedHammingBlockToLen = 8;

// Функции для вычисления длины, конвертации из size_t в вектор

size_t CreateFunctions::CalcCodedFilenameOrFileLenInBits(size_t hamming_block, size_t default_len) {
    size_t remainder = (kBitsInByte * default_len) % hamming_block;

    size_t result = (kBitsInByte * default_len / hamming_block) * (hamming_block + Hamming::CalcControlBits(hamming_block) + 1);

    if (remainder != 0) {
        result += (remainder + Hamming::CalcControlBits(remainder) + 1);
    }

    return result;
}

void CreateFunctions::ConvertSizeTToBytes(size_t object, std::vector<uint8_t>& bytes) {
    for (uint8_t i = 0; i < sizeof(size_t); ++i) {
        bytes.push_back((object >> (sizeof(size_t) * (kBitsInByte - 1 - i))) & 0xFF);
    }
}

// Запись длины в архив

void CreateFunctions::WriteLenToArchive(std::fstream& archive, size_t hamming_block, size_t default_len) {
    size_t coded_filename_or_file_len = CalcCodedFilenameOrFileLenInBits(hamming_block, default_len);

    std::vector<uint8_t> bytes;
    ConvertSizeTToBytes(coded_filename_or_file_len, bytes);

    IterationTools encode_tools;

    encode_tools.bits_count = hamming_block;
    encode_tools.bytes_count = hamming_block / kBitsInByte + (hamming_block % kBitsInByte != 0);

    while (encode_tools.i < kBytesInSizeT) {
        encode_tools.is_it_end = (encode_tools.i == kBytesInSizeT - 1);

        DecodeAndEncodeFunctions::IteratingOverDefaultSymbols(bytes[encode_tools.i], hamming_block, archive, encode_tools);
    }
}

// Запись имени файла в архив

void CreateFunctions::WriteFilenameToArchive(std::fstream& archive, const char* filename, size_t hamming_block) {
    IterationTools encode_tools;

    encode_tools.bits_count = hamming_block;
    encode_tools.bytes_count = hamming_block / kBitsInByte + (hamming_block % kBitsInByte != 0);

    while (filename[encode_tools.i] != '\0') {
        encode_tools.is_it_end = (filename[encode_tools.i + 1] == '\0');

        DecodeAndEncodeFunctions::IteratingOverDefaultSymbols(filename[encode_tools.i], hamming_block, archive, encode_tools);
    }
}

// Запись файла в архив

void CreateFunctions::WriteFileItselfToArchive(std::fstream& archive, const char* file_path, size_t hamming_block) {
    std::ifstream file(file_path, std::ios::binary);

    IterationTools encode_tools;

    encode_tools.bits_count = hamming_block;
    encode_tools.bytes_count = hamming_block / kBitsInByte + (hamming_block % kBitsInByte != 0);

    while (std::filesystem::file_size(std::filesystem::path(file_path)) > file.tellg()) {
        encode_tools.is_it_end = (std::filesystem::file_size(std::filesystem::path(file_path)) - file.tellg() == 1);

        char info = file.get();

        DecodeAndEncodeFunctions::IteratingOverDefaultSymbols(info, hamming_block, archive, encode_tools);
    }
}

// Запись файлов, их имен и длин

void CreateFunctions::WriteFilesToArchive(std::fstream& archive, const std::vector<const char*>& origin, const Arguments& parse_arguments) {
    for (size_t i = 0; i < origin.size(); ++i) {
        WriteLenToArchive(archive, kFixedHammingBlockToLen, std::strlen(origin[i])); // Зафиксируем блок для длины, чтобы было удобнее в дальнейшем декодировать файлы
        WriteFilenameToArchive(archive, origin[i], parse_arguments.hamming_block);

        WriteLenToArchive(archive, kFixedHammingBlockToLen, std::filesystem::file_size(origin[i])); // Также зафиксируем блок
        WriteFileItselfToArchive(archive, origin[i], parse_arguments.hamming_block);
    }
}