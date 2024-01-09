#pragma once

#include "Parser.h"
#include <cstdint>

class Requests {
public:
    Requests(const std::string& archive_name, size_t hamming_block);
    void CreateArchive(const std::vector<const char*>& initialize_filenames);
    void AppendFiles(const std::vector<const char*>& initialize_filenames);
    void ConcatenateArchives(const std::string& first_archive_to_concatenate, const std::string& second_archive_to_concatenate);
    void Extract(const std::vector<const char*>& extractable_files, bool is_extract_all_files);
    void Delete(const std::vector<const char*>& deletable_files);
    void ShowList();
    void AnalyzeArgs(const Arguments& parse_args);
private:
    const std::string& archive_name;
    size_t hamming_block;
};
