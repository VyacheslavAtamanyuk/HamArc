#include "Parser.h"

#include <cstring>
#include <cstdlib>
#include <vector>

void SetElements(int i, int argc, char** argv, std::vector<const char*>& container) {
    int iterate = i + 1;
    while (iterate < argc && argv[iterate][0] != '-') {
        container.push_back(argv[iterate]);
        ++iterate;
    }
}

Arguments Parse(int argc, char** argv) {
    Arguments args_for_archive;

    for (int i = 0; i < argc; ++i) {
        if (!std::strcmp(argv[i], "-f") || !std::strncmp(argv[i], "--file=", 7)) {
            if (!std::strcmp(argv[i], "-f")) {
                args_for_archive.archive_name = std::string(argv[i + 1]) + ".haf";
                SetElements(i + 1, argc, argv, args_for_archive.filenames_to_initialize);
            } else if (!std::strncmp(argv[i], "--file=", 7)) {
                args_for_archive.archive_name = std::string(argv[i] + 7) + ".haf";
                SetElements(i, argc, argv, args_for_archive.filenames_to_initialize);
            }
        }

        if (!std::strcmp(argv[i], "-с") || !std::strcmp(argv[i], "--create")) {
            args_for_archive.is_need_to_create = true;
        }

        if (!std::strcmp(argv[i], "-l") || !std::strcmp(argv[i], "--list")) {
            args_for_archive.is_show_list = true;
        }

        if (!std::strcmp(argv[i], "-A") || !std::strcmp(argv[i], "--concatenate")) {
            args_for_archive.is_concatenate = true;
            args_for_archive.first_archive_to_concatenate = std::string(argv[i + 1]) + ".haf";
            args_for_archive.second_archive_to_concatenate = std::string(argv[i + 2]) + ".haf";
        }

        if (!std::strcmp(argv[i], "-d") || !std::strcmp(argv[i], "--delete")) {
            args_for_archive.is_delete = true;

            SetElements(i, argc, argv, args_for_archive.deletable_files);
        }

        if (!std::strcmp(argv[i], "-x") || !std::strcmp(argv[i], "--extract")) {
            args_for_archive.is_extract = true;

            if (i + 1 == argc || argv[i + 1][0] == '-') {
                args_for_archive.is_extract_all_files = true;
            } else {
                SetElements(i, argc, argv, args_for_archive.extractable_files);
            }
        }

        if (!std::strcmp(argv[i], "-a") || !std::strcmp(argv[i], "--append")) {
            args_for_archive.is_need_to_add_something = true;

            SetElements(i, argc, argv, args_for_archive.filenames_to_append);
        }

        if (!std::strcmp(argv[i], "hamming_block")) {
            char* p_end;
            args_for_archive.hamming_block = std::strtol(argv[i + 1], &p_end, 10);
        }
    }

    return args_for_archive;
}