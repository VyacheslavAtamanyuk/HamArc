#ifndef LABWORK6_VYACHESLAVATAMANYUK_PARSER_H
#define LABWORK6_VYACHESLAVATAMANYUK_PARSER_H

#include <vector>
#include <string>

struct Arguments {
    std::string archive_name;

    bool need_to_create = false;
    bool is_show_list = false;
    bool is_concatenate = false;

    std::string first_archive_for_concatenate;
    std::string second_archive_for_concatenate;

    bool need_to_add_something = false;

    std::vector<const char*> initialize_file_names;
    std::vector<const char*> append_file_names;

    bool is_delete = false;
    std::vector<const char*> deletable_files;

    bool is_extract = false;
    bool extract_all_files = false;
    std::vector<const char*> extractable_files;

    size_t hamming_block;
};

Arguments Parse(int argc, char** argv);

#endif //LABWORK6_VYACHESLAVATAMANYUK_PARSER_H
