#include "Parser.h"
#include <cstring>
#include <cstdlib>
#include <fstream>

Arguments Parse(int argc, char** argv) {
    Arguments args_for_archive;
    for (int i = 0; i < argc; ++i) {
        if (std::strcmp(argv[i], "-f") == 0 || std::strncmp(argv[i], "--file=", 7) == 0) {
            if (std::strcmp(argv[i], "-f") == 0) {
                args_for_archive.archive_name = std::string(argv[i+1]) + ".haf";
            } else if (std::strncmp(argv[i], "--file=", 7) == 0) {
                args_for_archive.archive_name = std::string(argv[i] + 7) + ".haf";
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
            args_for_archive.first_archive_for_concatenate = std::string(argv[i + 1]) + ".haf";
            args_for_archive.second_archive_for_concatenate = std::string(argv[i + 2]) + ".haf";
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
                while (j < argc && argv[j][0] != '-') {
                    args_for_archive.extractable_files.push_back(argv[j]);
                    ++j;
                }
            }
        }
        if (std::strcmp(argv[i], "-a") == 0 || std::strcmp(argv[i], "--append") == 0) {
            args_for_archive.need_to_add_something = true;
        }

        if (std::strcmp(argv[i], "hamming_argument") == 0) {
            char* p_end;
            args_for_archive.hamming_block = std::strtol(argv[i+1], &p_end, 10);
        }

        if (argv[i][0] != '-' && i > 0 && std::strncmp(argv[i - 1], "-f", 2) != 0 && std::strncmp(argv[i - 1], "--file=", 7) != 0) {
            std::fstream file;
            file.open(argv[i], std::ios::in);
            if (file.is_open()) {
                if (args_for_archive.need_to_create) {
                    args_for_archive.initialize_file_names.push_back(argv[i]);
                } else if (args_for_archive.need_to_add_something) {
                    args_for_archive.append_file_names.push_back(argv[i]);
                }
            }
        }
    }
    return args_for_archive;
}