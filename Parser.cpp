#include "Parser.h"
#include <cstring>
#include <cstdlib>

Arguments Parse(int argc, char** argv) {
    Arguments args_for_archive;
    for (int i = 0; i < argc; ++i) {
        if (std::strcmp(argv[i], "-f") == 0 || std::strcmp(argv[i], "--file=") > 0) {
            if (std::strcmp(argv[i], "-f") == 0) {
                args_for_archive.archive_name = argv[i+1];
            } else if (std::strcmp(argv[i], "--file=") > 0) {
                args_for_archive.archive_name = argv[i] + 7;
            }
        }
        if (std::strcmp(argv[i], "-с") == 0 || std::strcmp(argv[i], "--create") == 0) {
            args_for_archive.need_to_create = true;
        }
        if (std::strcmp(argv[i], "-l") == 0 || std::strcmp(argv[i], "--list") == 0) {
            args_for_archive.request_for_list = true;
        }
        if (std::strcmp(argv[i], "-A") == 0 || std::strcmp(argv[i], "--concatenate") == 0) {
            args_for_archive.is_concatenate = true;
            args_for_archive.first_archive_for_concatenate = argv[i+1];
            args_for_archive.second_archive_for_concatenate = argv[i+2];
        }
        if (std::strcmp(argv[i], "-d") == 0 || std::strcmp(argv[i], "--delete") == 0) {
            args_for_archive.delete_files = true;
            args_for_archive.deletable_files.push_back(argv[i+1]);
        }
        if (std::strcmp(argv[i], "-x") == 0 || std::strcmp(argv[i], "--extract") == 0) {
            args_for_archive.extract = true;
            if (argv[i+1][0] == '-') {
                args_for_archive.extract_all_files = true;
            } else {
                int j = i+1;
                while (argv[j][0] != '-' && j < argc) {
                    args_for_archive.extractable_files.push_back(argv[j]);
                    ++j;
                }
            }
        }
        if (std::strcmp(argv[i], "-a") == 0 || std::strcmp(argv[i], "--append") == 0) {
            args_for_archive.append_file_names.push_back(argv[i+1]);
            args_for_archive.need_to_add_something = true;
        }

        if (std::strcmp(argv[i], "hemming_argument") == 0) {
            char* p_end;
            args_for_archive.hamming_block = std::strtol(argv[i+1], &p_end, 10);
        }
        if (argv[i][0] != '-') {

        }
    }
    return args_for_archive;
}