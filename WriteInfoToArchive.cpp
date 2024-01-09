#include "Requests.h"
#include "WriteInfoToArchive.h"
#include "CommonTools.h"
#include "DecodeAndEncode.h"

const uint8_t kBytesInSizeT = 8;
const uint8_t kBitsInByte = 8;
const uint8_t kFixedHammingBlockToLen = 8;

void ConvertSizeTToBytes(size_t object, uint8_t* bytes) {
    for (uint8_t i = 0; i < kBytesInSizeT; ++i) {
        bytes[i] = (object >> (kBytesInSizeT * (kBitsInByte - 1 - i))) & 0xFF;
    }
}

WriteInfoToArchive::WriteInfoToArchive(std::ofstream& archive, const std::string& archive_name, size_t hamming_block): archive(archive), archive_name(archive_name), hamming_block(hamming_block) {}

// Запись длины закодированного файла / имени файла в архив

void WriteInfoToArchive::WriteLenBytesToArchive(size_t default_len) {
    size_t coded_filename_or_file_len = CommonTools::CalcCodedFilenameOrFileLenInBits(kFixedHammingBlockToLen, default_len); // Зафиксируем блок для возможности однозначного кодирования / декодирования длины

    uint8_t bytes[kBytesInSizeT];
    ConvertSizeTToBytes(coded_filename_or_file_len, bytes);

    IterationTools encode_tools(hamming_block);
    DecodeAndEncode iterator;

    while (encode_tools.i < kBytesInSizeT) {
        encode_tools.is_it_end = (encode_tools.i == kBytesInSizeT - 1);

        iterator.EncodeBytesAndPutToArchive(bytes[encode_tools.i], hamming_block, archive, encode_tools);
    }
}

// Запись имени файла в архив

void WriteInfoToArchive::WriteFilenameBytesToArchive(const char* filename) {
    IterationTools encode_tools(hamming_block);
    DecodeAndEncode iterator;

    while (filename[encode_tools.i] != '\0') {
        encode_tools.is_it_end = (filename[encode_tools.i + 1] == '\0');

        iterator.EncodeBytesAndPutToArchive(filename[encode_tools.i], hamming_block, archive, encode_tools);
    }
}

// Запись файла в архив

void WriteInfoToArchive::WriteFileItselfBytesToArchive(const char* file_path) {
    std::ifstream file(file_path, std::ios::binary);

    IterationTools encode_tools(hamming_block);
    DecodeAndEncode iterator;

    while (std::filesystem::file_size(std::filesystem::path(file_path)) > file.tellg()) {
        encode_tools.is_it_end = (std::filesystem::file_size(std::filesystem::path(file_path)) - file.tellg() == 1);
        char info = file.get();

        iterator.EncodeBytesAndPutToArchive(info, hamming_block, archive, encode_tools);
    }
}

// Запись файлов, их имен и длин

void WriteInfoToArchive::WriteFilesToArchive(const std::vector<const char*>& origin) {
    for (size_t i = 0; i < origin.size(); ++i) {
        WriteLenBytesToArchive(std::strlen(origin[i]));
        WriteFilenameBytesToArchive(origin[i]);

        WriteLenBytesToArchive(std::filesystem::file_size(origin[i]));
        WriteFileItselfBytesToArchive(origin[i]);
    }
}