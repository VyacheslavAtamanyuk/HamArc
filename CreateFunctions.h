#ifndef LABWORK6_CREATEFUNCTIONS_H
#define LABWORK6_CREATEFUNCTIONS_H

#include "Parser.h"
#include "HammingCode.h"

#include <cstdint>
#include <vector>
#include <fstream>
#include <cstring>
#include <filesystem>

class CreateFunctions {
public:
    static size_t CalcCodedFilenameOrFileLenInBits(size_t hamming_block, size_t default_len);
    static void ConvertSizeTToBytes(size_t object, std::vector<uint8_t>& bytes);
    static void WriteLenToArchive(std::fstream& archive, size_t hamming_block, size_t default_len);
    static void WriteFilenameToArchive(std::fstream& archive, const char* filename, size_t hamming_block);
    static void WriteFileItselfToArchive(std::fstream& archive, const char* file_path, size_t hamming_block);
    static void WriteFilesToArchive(std::fstream& archive, const std::vector<const char*>& origin, const Arguments& parse_arguments);
};

#endif //LABWORK6_CREATEFUNCTIONS_H
