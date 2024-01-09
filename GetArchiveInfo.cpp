#include "GetArchiveInfo.h"
#include "HammingCode.h"
#include "CommonTools.h"

#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>

const uint8_t kBitsInByte = 8;
const uint8_t kCodedLenSizeInBytes = 13;
const uint8_t kFixedHammingBlockLenSize = 8;

// Задание параметров рассматриваемого архива: путь к нему и блок Хемминга для декодирования файлов

GetArchiveInfo::GetArchiveInfo(std::ifstream& archive, const std::string& archive_name, size_t hamming_block): archive(archive), archive_name(archive_name), hamming_block(hamming_block) {}

// Вывод всех имен файлов

void GetArchiveInfo::GetAllFilenames() {
    CommonTools auxiliary_tools(archive, archive_name, hamming_block);
    std::string original_filename;

    while (std::filesystem::file_size(std::filesystem::path(archive_name)) > archive.tellg()) {
        auxiliary_tools.GetNextFilename(original_filename);

        std::cout << original_filename << '\n';

        original_filename.clear();
        auxiliary_tools.Skip();
    }
}

void GetArchiveInfo::GetExtractableFiles(const std::vector<const char*>& extractable_files, bool is_extract_all_files) {
    CommonTools auxiliary_tools(archive, archive_name, hamming_block);
    std::string original_filename;

    while (std::filesystem::file_size(std::filesystem::path(archive_name)) > archive.tellg()) {
        auxiliary_tools.GetNextFilename(original_filename);

        if (!auxiliary_tools.IsExtractableFile(extractable_files, original_filename) && !is_extract_all_files) {
            original_filename.clear();
            auxiliary_tools.Skip();
            continue;
        }

        std::ofstream extractable_file(original_filename, std::ios::binary);
        auxiliary_tools.GetNextFile(extractable_file);
    }
}

void GetArchiveInfo::LeaveOnlyUndeletableFiles(const std::vector<const char*>& deletable_files) {
    CommonTools auxiliary_tools(archive, archive_name, hamming_block);
    std::string original_filename;

    std::ofstream updated_archive(archive_name + "temp", std::ios::binary);

    while (std::filesystem::file_size(std::filesystem::path(archive_name)) > archive.tellg()) {
        auxiliary_tools.GetNextFilename(original_filename);

        if (auxiliary_tools.IsDeletableFile(deletable_files, original_filename)) {
            original_filename.clear();
            auxiliary_tools.Skip();
            continue;
        }

        auxiliary_tools.WriteEncodedLenAndFilenameBytes(original_filename.size(), updated_archive);
        auxiliary_tools.WriteEncodedLenAndFileBytes(updated_archive);

        original_filename.clear();
    }

    archive.close();
    updated_archive.close();

    std::remove((archive_name).c_str());
    std::rename((archive_name + "temp").c_str(), (archive_name).c_str());
}