#pragma once

#include <vector>
#include <string>

struct Arguments {
    std::string archive_name;

    bool is_need_to_create = false;
    bool is_show_list = false;
    bool is_concatenate = false;

    std::string first_archive_to_concatenate;
    std::string second_archive_to_concatenate;

    bool is_need_to_add_something = false;

    std::vector<const char*> filenames_to_initialize;
    std::vector<const char*> filenames_to_append;

    bool is_delete = false;
    std::vector<const char*> deletable_files;

    bool is_extract = false;
    bool is_extract_all_files = false;
    std::vector<const char*> extractable_files;

    size_t hamming_block;
};

Arguments Parse(int argc, char** argv);
