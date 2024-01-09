#include "Requests.h"
#include "Parser.h"

#include "WriteInfoToArchive.h"
#include "GetArchiveInfo.h"

#include <fstream>
#include <iostream>
#include <filesystem>

const uint8_t kBitsInByte = 8;
const uint8_t kCodedLenSizeInBytes = 13;
const uint8_t kFixedHammingBlockLenSize = 8;

Requests::Requests(const std::string& archive_name, size_t hamming_block): archive_name(archive_name), hamming_block(hamming_block) {}

void Requests::CreateArchive(const std::vector<const char*>& initialize_filenames) {
    std::ofstream new_archive(archive_name, std::ios::binary);

    if (new_archive.is_open()) {
        WriteInfoToArchive new_files_to_initialize_archive(new_archive, archive_name, hamming_block);
        new_files_to_initialize_archive.WriteFilesToArchive(initialize_filenames);
    }
}

void Requests::AppendFiles(const std::vector<const char*>& append_filenames) {
    std::ofstream archive(archive_name, std::ios::app | std::ios::ate | std::ios::binary);

    if (archive.is_open()) {
        WriteInfoToArchive appendable_files_to_existing_archive(archive, archive_name, hamming_block);
        appendable_files_to_existing_archive.WriteFilesToArchive(append_filenames);
    }
}

void Requests::ConcatenateArchives(const std::string& first_archive_to_concatenate, const std::string& second_archive_to_concatenate) {
    std::ifstream first_archive(first_archive_to_concatenate, std::ios::binary);

    std::ifstream second_archive(second_archive_to_concatenate, std::ios::binary);

    std::ofstream result_archive(archive_name, std::ios::binary);

    if (first_archive.is_open() && second_archive.is_open() && result_archive.is_open()) {
        while (std::filesystem::file_size(std::filesystem::path(first_archive_to_concatenate)) > first_archive.tellg()) {
            char info = first_archive.get();
            result_archive << info;
        }

        while (std::filesystem::file_size(std::filesystem::path(second_archive_to_concatenate)) > second_archive.tellg()) {
            char info = second_archive.get();
            result_archive << info;
        }
    }
}

void Requests::Extract(const std::vector<const char*>& extractable_files, bool is_extract_all_files) {
    std::ifstream archive(archive_name, std::ios::binary);

    if (archive.is_open()) {
        GetArchiveInfo my_archive(archive, archive_name, hamming_block);
        my_archive.GetExtractableFiles(extractable_files, is_extract_all_files);
    }
}

void Requests::Delete(const std::vector<const char*>& deletable_files) {
    std::ifstream archive(archive_name, std::ios::binary);

    if (archive.is_open()) {
        GetArchiveInfo my_archive(archive, archive_name, hamming_block);
        my_archive.LeaveOnlyUndeletableFiles(deletable_files);
    }
}

void Requests::ShowList() {
    std::ifstream archive(archive_name, std::ios::binary);

    if (archive.is_open()) {
        GetArchiveInfo my_archive(archive, archive_name, hamming_block);
        my_archive.GetAllFilenames();
    }
}

void Requests::AnalyzeArgs(const Arguments& parse_arguments) {
    if (parse_arguments.is_need_to_create) {
        CreateArchive(parse_arguments.filenames_to_initialize);
    }

    if (parse_arguments.is_need_to_add_something) {
        AppendFiles(parse_arguments.filenames_to_append);
    }

    if (parse_arguments.is_concatenate) {
        ConcatenateArchives(parse_arguments.first_archive_to_concatenate, parse_arguments.second_archive_to_concatenate);
    }

    if (parse_arguments.is_extract) {
        Extract(parse_arguments.extractable_files, parse_arguments.is_extract_all_files);
    }

    if (parse_arguments.is_delete) {
        Delete(parse_arguments.deletable_files);
    }

    if (parse_arguments.is_show_list) {
        ShowList();
    }
}