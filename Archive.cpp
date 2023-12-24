#include "Archive.h"
#include "Parser.h"
#include "HammingCode.h"

#include <cstring>
#include <fstream>
#include <vector>
#include <iostream>

const uint8_t kBitsInByte = 8;
const uint8_t kBytesInEachFilename = 15;
const uint8_t kBytesInLenOfHammingCodeFile = 8;

// Вспомогательные функции


size_t GetNextEightBytes(std::fstream& archive) {
    char info;
    size_t result = 0;
    for (uint8_t i = 0; i < kBitsInByte; ++i) {
        archive.get(info);
        result |= (info << (kBitsInByte - 1 - i));
    }
    return result;
}

char* GetNextFiveteenBytes(std::fstream& archive, char* ptr) {
    char info;
    for (uint8_t i = 0; i < kBytesInEachFilename; ++i) {
        archive.get(info);
        ptr[i] = info;
    }
    return ptr;
}

void WriteFiles(std::fstream& archive, const std::vector<const char*>& origin, const Arguments& parse_arguments) {
    for (size_t i = 0; i < origin.size(); ++i) {
        for (size_t j = 0; j < origin[i][j]; ++j) {
            archive.write(reinterpret_cast<char*>(origin[i][j]), 1);
        }

        std::vector<uint8_t> hamming_code_file;
        size_t len_of_hamming_code_file = Hamming::HammingCode(hamming_code_file, parse_arguments.hamming_block, origin[i], true);

        archive.write(reinterpret_cast<char*>(len_of_hamming_code_file), 8);
        for (size_t j = 0; j < hamming_code_file.size(); ++j) {
            archive.write(reinterpret_cast<char*>(hamming_code_file[i]), 1);
        }
    }
}

void CreateFile(char* filename, std::vector<uint8_t>& bytes) {
    std::fstream new_file;
    new_file.open(filename, std::ios::out);

    for (size_t i = 0; i < bytes.size(); ++i) {
        new_file.write(reinterpret_cast<char*>(bytes[i]), 1);
    }

    new_file.close();
}

void ExtractFile(std::fstream& archive, const char* search, size_t hamming_block, bool extract_all_files) {
    std::vector<uint8_t> data;
    std::vector<uint8_t> decode_data;

    while (!archive.eof()) {
        char* filename = new char[15];
        filename = GetNextFiveteenBytes(archive, filename);

        size_t size = GetNextEightBytes(archive);

        bool flag = false;
        if (!std::strcmp(filename, search) && !extract_all_files) {
            archive.seekg(size, std::ios_base::cur);
            flag = true;
            continue;
        }

        char info;
        for(size_t i = 0; i < size; ++i) {
            archive.get(info);
            data.push_back(info);
        }

        decode_data = Hamming::HammingDecode(data, hamming_block);
        CreateFile(filename, decode_data);

        decode_data.clear();
        data.clear();

        if (flag && !extract_all_files) {
            break;
        }
    }
}

bool IsDeletableFile(const std::vector<const char*>& deletable_files, const char* filename) {
    for (size_t i = 0; i < deletable_files.size(); ++i) {
        if (!strcmp(deletable_files[i], filename)) {
            return true;
        }
    }
    return false;
}

void WriteInNewData(std::vector<uint8_t> new_data, const char* filename, size_t size, const std::vector<uint8_t>& data) {
    for(uint8_t i = 0; i < kBytesInEachFilename; ++i) {
        new_data.push_back(filename[i]);
    }

    for(uint8_t i = 0; i < kBitsInByte; ++i) {
        new_data.push_back(size >> (kBitsInByte - 1 - i));
    }

    for (size_t i = 0; i < size; ++i) {
        new_data.push_back(data[i]);
    }
}

// Основные функции

void Archive::CreateArchive(const Arguments& parse_arguments) {
    std::fstream new_archive;
    new_archive.open(parse_arguments.archive_name, std::ios::out);

    WriteFiles(new_archive, parse_arguments.initialize_file_names, parse_arguments);

    new_archive.close();
}

void Archive::AppendFiles(const Arguments& parse_arguments) {
    std::fstream archive;
    archive.open(parse_arguments.archive_name, std::ios::app | std::ios::ate);

    WriteFiles(archive, parse_arguments.append_file_names, parse_arguments);

    archive.close();
}

void Archive::ShowList(const Arguments& parse_arguments) {
    std::fstream archive;
    archive.open(parse_arguments.archive_name, std::ios::in);

    while (!archive.eof()) {
        char* filename = new char[15];
        filename = GetNextFiveteenBytes(archive, filename);
        std::cout << filename << '\n';

        size_t size = GetNextEightBytes(archive);
        archive.seekg(size, std::ios_base::cur);

        delete[] filename;
        filename = nullptr;
    }

    archive.close();
}

void Archive::ConcatenateArchives(const Arguments& parse_arguments) {
    std::fstream first_archive;
    first_archive.open(parse_arguments.first_archive_for_concatenate, std::ios::in);

    std::fstream second_archive;
    second_archive.open(parse_arguments.second_archive_for_concatenate, std::ios::in);

    std::fstream result_archive;
    result_archive.open(parse_arguments.archive_name, std::ios::out);

    while (!first_archive.eof()) {
        char info;
        first_archive.get(info);
        result_archive.write(reinterpret_cast<char*>(info), 1);
    }

    while (!second_archive.eof()) {
        char info;
        second_archive.get(info);
        result_archive.write(reinterpret_cast<char*>(info), 1);
    }

    first_archive.close();
    second_archive.close();
    result_archive.close();
}

void Archive::Extract(const Arguments& parse_arguments) {
    std::fstream archive;
    archive.open(parse_arguments.archive_name, std::ios::in);

    if (parse_arguments.extract_all_files) {
        ExtractFile(archive, "", parse_arguments.hamming_block, true);
    } else {
        for (size_t i = 0; i < parse_arguments.extractable_files.size(); ++i) {
            ExtractFile(archive, parse_arguments.extractable_files[i], parse_arguments.hamming_block, false);
        }
    }

    archive.close();
}

void Archive::Delete(const Arguments& parse_arguments) {
    std::fstream archive;
    archive.open(parse_arguments.archive_name, std::ios::in);

    std::vector<uint8_t> new_data;
    std::vector<uint8_t> data;
    while (!archive.eof()) {
        char* filename = new char[15];
        filename = GetNextFiveteenBytes(archive, filename);

        size_t size = GetNextEightBytes(archive);

        char info;
        for (size_t i = 0; i < size; ++i) {
            archive.get(info);
            data.push_back(info);
        }

        if (IsDeletableFile(parse_arguments.deletable_files, filename)) {
            continue;
        }

        WriteInNewData(new_data, filename, size, data);
    }

    archive.close();

    archive.open(parse_arguments.archive_name, std::ios::out);

    for (size_t i = 0; i < new_data.size(); ++i) {
        archive.write(reinterpret_cast<char*>(new_data[i]), 1);
    }

    archive.close();
}

// Анализ поданных в консоли аргументов

void AnaliseArgs(const Arguments& parse_arguments) {
    if (parse_arguments.need_to_create) {
        Archive::CreateArchive(parse_arguments);
    }
    if (parse_arguments.need_to_add_something) {
        Archive::AppendFiles(parse_arguments);
    }
    if (parse_arguments.request_for_list) {
        Archive::ShowList(parse_arguments);
    }
    if (parse_arguments.is_concatenate) {
        Archive::ConcatenateArchives(parse_arguments);
    }
    if (parse_arguments.extract) {
        Archive::Extract(parse_arguments);
    }
    if (parse_arguments.delete_files) {
        Archive::Delete(parse_arguments);
    }
}