#pragma once

#include "Parser.h"
#include "HammingCode.h"

#include <cstdint>
#include <vector>
#include <fstream>
#include <cstring>
#include <filesystem>

class WriteInfoToArchive {
public:
    WriteInfoToArchive(std::ofstream& archive, const std::string& archive_name, size_t hamming_block);

    void WriteLenBytesToArchive(size_t default_len);
    void WriteFilenameBytesToArchive(const char* filename);
    void WriteFileItselfBytesToArchive(const char* file_path);
    void WriteFilesToArchive(const std::vector<const char*>& origin);
private:
    std::ofstream& archive;
    const std::string& archive_name;
    size_t hamming_block;
};
