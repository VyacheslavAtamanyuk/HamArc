#pragma once

#include "GetArchiveInfo.h"
#include <fstream>
#include <cstdint>
#include <vector>

class CommonTools {
public:
    CommonTools(std::ifstream& archive, const std::string& archive_name, size_t hamming_block);

    size_t GetLen();
    void GetNextFilename(std::string& original_filename);
    void GetNextFile(std::ofstream& file);
    void Skip();

    void WriteEncodedLenAndFilenameBytes(size_t original_filename_len, std::ofstream& file);
    void WriteEncodedLenAndFileBytes(std::ofstream& file);

    static size_t CalcCodedFilenameOrFileLenInBits(size_t hamming_block, size_t default_len);
    static bool IsExtractableFile(const std::vector<const char*>& extractable_files, const std::string& filename);
    static bool IsDeletableFile(const std::vector<const char*>& deletable_files, const std::string& filename);
private:
    std::ifstream& archive;
    const std::string& archive_name;
    size_t hamming_block;
};
