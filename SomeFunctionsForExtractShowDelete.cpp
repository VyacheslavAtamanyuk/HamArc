#include "SomeFunctionsForExtractShowDelete.h"
#include "SomeGeneralFunctions.h"
#include "Parser.h"

#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>

const uint8_t kBitsInByte = 8;
const uint8_t kCodedLenSizeInBytes = 13;
const uint8_t kFixedHammingBlockLenSize = 8;

// Очистка параметров у объекта структуры VectorTools

void VectorAndPointerTools::clear() {
    coded_len_bytes.clear();
    decoded_len_bytes.clear();

    coded_filename_bytes.clear();
    decoded_filename_bytes.clear();

    coded_file_bytes.clear();
    decoded_file_bytes.clear();

    delete[] origin_filename;
    origin_filename = nullptr;
}

// Создать файл с данным именем и байтами

void SomeFunctionsForExtractShowDelete::CreateFile(const char* filename, const std::vector<uint8_t>& bytes) {
    std::ofstream new_file(filename, std::ios::binary);

    for (size_t i = 0; i < bytes.size(); ++i) {
        new_file << bytes[i];
    }

    new_file.close();
}

// Проверка на то, надо ли извлекать файл

bool SomeFunctionsForExtractShowDelete::IsExtractableFile(const std::vector<const char*>& extractable_files, const char* filename) {
    for (size_t i = 0; i < extractable_files.size(); ++i) {
        if (!strcmp(extractable_files[i], filename)) {
            return true;
        }
    }
    return false;
}

// Проверка на то, надо ли удалять файл

bool SomeFunctionsForExtractShowDelete::IsDeletableFile(const std::vector<const char*>& deletable_files, const char* filename) {
    for (size_t i = 0; i < deletable_files.size(); ++i) {
        if (!strcmp(deletable_files[i], filename)) {
            return true;
        }
    }
    return false;
}

// Запись закодированной длины закодированного имени файла/файла и закодированного имени файла/файла в вектор

void SomeFunctionsForExtractShowDelete::WriteInNewData(std::vector<uint8_t>& new_data, const std::vector<uint8_t>& coded_len_bytes, const std::vector<uint8_t>& coded_filename_or_file_bytes) {
    for (size_t i = 0; i < coded_len_bytes.size(); ++i) {
        new_data.push_back(coded_len_bytes[i]);
    }

    for (size_t i = 0; i < coded_filename_or_file_bytes.size(); ++i) {
        new_data.push_back(coded_filename_or_file_bytes[i]);
    }
}

// Функция для извлечения, вывода имен и удаления файлов

void SomeFunctionsForExtractShowDelete::EitherExtractOrShowOrDeleteFile(std::ifstream& archive, const std::string& archive_path, size_t hamming_block, bool is_delete, bool is_show, bool is_extract, bool extract_all_files, const std::vector<const char*>& deletable_files, const std::vector<const char*>& extractable_files) {

    VectorAndPointerTools container_tools;

    std::vector<uint8_t> new_data_to_write;  // Используется при удалении

    while (std::filesystem::file_size(std::filesystem::path(archive_path)) > archive.tellg()) {
        SomeGeneralFunctions::GetNextNthDecodedBytes(archive, archive_path, hamming_block, kCodedLenSizeInBytes, container_tools.coded_len_bytes, container_tools.decoded_len_bytes);
        size_t len_in_bits = SomeGeneralFunctions::ConvertBytesToSizeT(container_tools.decoded_len_bytes);
        size_t len_in_bytes = len_in_bits / kBitsInByte + (len_in_bits % kBitsInByte != 0);

        SomeGeneralFunctions::GetNextNthDecodedBytes(archive, archive_path, hamming_block, len_in_bytes, container_tools.coded_filename_bytes, container_tools.decoded_filename_bytes);

        char* original_filename = new char[container_tools.decoded_filename_bytes.size() + 1];
        SomeGeneralFunctions::GetPointerOnCharFromDecodedObject(container_tools.decoded_filename_bytes, original_filename);

        if (is_extract || is_delete) {
            if ((is_extract && !IsExtractableFile(extractable_files, original_filename) && !extract_all_files) || (is_delete && IsDeletableFile(deletable_files, original_filename))) {
                SomeGeneralFunctions::Skip(archive, archive_path, hamming_block);

                container_tools.clear();

                continue;
            }

            if (is_delete) {
                WriteInNewData(new_data_to_write, container_tools.coded_len_bytes, container_tools.coded_filename_bytes);
            }

            container_tools.coded_len_bytes.clear();
            container_tools.decoded_len_bytes.clear();

            SomeGeneralFunctions::GetNextNthDecodedBytes(archive, archive_path, hamming_block, kCodedLenSizeInBytes, container_tools.coded_len_bytes, container_tools.decoded_len_bytes);
            len_in_bits = SomeGeneralFunctions::ConvertBytesToSizeT(container_tools.decoded_len_bytes);
            len_in_bytes = len_in_bits / kBitsInByte + (len_in_bits % kBitsInByte != 0);

            SomeGeneralFunctions::GetNextNthDecodedBytes(archive, archive_path, hamming_block, len_in_bytes, container_tools.coded_file_bytes, container_tools.decoded_file_bytes);

            if (is_delete) {
                WriteInNewData(new_data_to_write, container_tools.coded_len_bytes, container_tools.coded_file_bytes);
            }
            if (is_extract) {
                CreateFile(original_filename, container_tools.decoded_file_bytes);
            }
        } else if (is_show) {
            std::cout << original_filename << '\n';

            SomeGeneralFunctions::Skip(archive, archive_path, hamming_block);
        }

        container_tools.clear();
    }

    archive.close();

    if (is_delete) {
        std::ofstream updated_archive(archive_path, std::ios::binary);

        for (size_t i = 0; i < new_data_to_write.size(); ++i) {
            updated_archive << new_data_to_write[i];
        }

        updated_archive.close();
    }
}