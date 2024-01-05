#include "Archive.h"
#include "Parser.h"

#include "CreateFunctions.h"
#include "SomeFunctionsForExtractShowDelete.h"

#include <fstream>
#include <iostream>
#include <filesystem>

const uint8_t kBitsInByte = 8;
const uint8_t kCodedLenSizeInBytes = 13;
const uint8_t kFixedHammingBlockLenSize = 8;

// Основные функции

void Archive::CreateArchive(const Arguments& parse_arguments) {
    std::fstream new_archive(parse_arguments.archive_name, std::ios::out | std::ios::binary);

    if (new_archive.is_open()) {
        CreateFunctions::WriteFilesToArchive(new_archive, parse_arguments.initialize_file_names, parse_arguments);

        new_archive.close();
    }
}

void Archive::AppendFiles(const Arguments& parse_arguments) {
    std::fstream archive(parse_arguments.archive_name, std::ios::app | std::ios::ate | std::ios::binary);

    if (archive.is_open()) {
        CreateFunctions::WriteFilesToArchive(archive, parse_arguments.append_file_names, parse_arguments);

        archive.close();
    }
}

void Archive::ConcatenateArchives(const Arguments& parse_arguments) {
    std::ifstream first_archive(parse_arguments.first_archive_for_concatenate, std::ios::binary);

    std::ifstream second_archive(parse_arguments.second_archive_for_concatenate, std::ios::binary);

    std::ofstream result_archive(parse_arguments.archive_name, std::ios::binary);

    if (first_archive.is_open() && second_archive.is_open() && result_archive.is_open()) {
        while (std::filesystem::file_size(std::filesystem::path(parse_arguments.first_archive_for_concatenate)) > first_archive.tellg()) {
            char info = first_archive.get();
            result_archive << info;
        }

        while (std::filesystem::file_size(std::filesystem::path(parse_arguments.second_archive_for_concatenate)) > second_archive.tellg()) {
            char info = second_archive.get();
            result_archive << info;
        }

        first_archive.close();
        second_archive.close();
        result_archive.close();
    }
}

void Archive::Extract(const Arguments& parse_arguments) {
    std::ifstream archive(parse_arguments.archive_name, std::ios::binary);

    if (archive.is_open()) {
        SomeFunctionsForExtractShowDelete::EitherExtractOrShowOrDeleteFile(archive, parse_arguments.archive_name, parse_arguments.hamming_block, false, false, true, parse_arguments.extract_all_files, parse_arguments.deletable_files, parse_arguments.extractable_files);
    }
}

void Archive::ShowList(const Arguments& parse_arguments) {
    std::ifstream archive(parse_arguments.archive_name, std::ios::binary);

    if (archive.is_open()) {
        SomeFunctionsForExtractShowDelete::EitherExtractOrShowOrDeleteFile(archive, parse_arguments.archive_name, parse_arguments.hamming_block, false, true, false, false, parse_arguments.deletable_files, parse_arguments.extractable_files);
    }
}


void Archive::Delete(const Arguments& parse_arguments) {
    std::ifstream archive(parse_arguments.archive_name, std::ios::binary);

    if (archive.is_open()) {
        SomeFunctionsForExtractShowDelete::EitherExtractOrShowOrDeleteFile(archive, parse_arguments.archive_name, parse_arguments.hamming_block, true, false, false, false, parse_arguments.deletable_files, parse_arguments.extractable_files);
    }
}

// Анализ поданных в консоли аргументов

void AnalyzeArgs(const Arguments& parse_arguments) {
    if (parse_arguments.need_to_create) {
        Archive::CreateArchive(parse_arguments);
    }

    if (parse_arguments.need_to_add_something) {
        Archive::AppendFiles(parse_arguments);
    }

    if (parse_arguments.is_concatenate) {
        Archive::ConcatenateArchives(parse_arguments);
    }

    if (parse_arguments.is_extract) {
        Archive::Extract(parse_arguments);
    }

    if (parse_arguments.is_show_list) {
        Archive::ShowList(parse_arguments);
    }

    if (parse_arguments.is_delete) {
        Archive::Delete(parse_arguments);
    }
}