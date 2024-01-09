#pragma once

#include <cstdint>
#include <vector>
#include <fstream>

class GetArchiveInfo {
public:
    GetArchiveInfo(std::ifstream& archive, const std::string& archive_name, size_t hamming_block);

    void GetAllFilenames();
    void GetExtractableFiles(const std::vector<const char*>& extractable_files, bool is_extract_all_files);
    void LeaveOnlyUndeletableFiles(const std::vector<const char*>& deletable_files);
private:
    std::ifstream& archive;
    const std::string& archive_name;
    size_t hamming_block;
};
