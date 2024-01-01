#include "Archive.h"
#include "Parser.h"
#include "HammingCode.h"

#include <cstring>
#include <fstream>
#include <vector>
#include <iostream>
#include <filesystem>

const uint8_t kBitsInByte = 8;
const uint8_t kCodedLenSizeInBytes = 13;

// Берем следующие n байт в файле

void GetNextNthBytes(std::fstream& archive, const std::string& archive_path, size_t n, std::vector<uint8_t>& object) {
    size_t i = 0;
    while ((std::filesystem::file_size(std::filesystem::path(archive_path)) > archive.tellg()) && i < n) {
        char info = archive.get();
        object.push_back(info);
        ++i;
    }
}

// Вспомогательные функции для записи файлов в архив

void ConvertSizeTToBits(std::vector<bool>& converted_len, size_t len) {
    for (uint8_t i = 0; i < kBitsInByte * sizeof(size_t); ++i) {
        converted_len.push_back((len >> (kBitsInByte * sizeof(size_t) - 1 - i)) & 1);
    }
}

void ConvertBitsToBytes(std::vector<bool>& bits, std::vector<uint8_t>& bytes) {
    for (size_t i = 0; i < (bits.size() / kBitsInByte); ++i) {
        uint8_t info = 0;
        for (uint8_t j = 0; j < kBitsInByte; ++j) {
            info |= (bits[i * kBitsInByte + j] << (kBitsInByte - 1 - j));
        }
        bytes.push_back(info);
    }

    if (bits.size() % kBitsInByte != 0) {
        uint8_t info = 0;
        for (uint8_t i = 0; i < bits.size() % kBitsInByte; ++i) {
            info |= (bits[(bits.size() / kBitsInByte) * kBitsInByte + i] << (kBitsInByte - 1 - i)); // Если кол-во бит не делится на 8, то паддинг у остатка будет делать в конец (так удобнее декодировать файл)
        }
        bytes.push_back(info);
    }
}

// Считаем биты файла

void GetOriginFileBits(std::vector<bool>& bits, std::fstream& file) {
    char info;
    while (file.get(info)) {
        for (uint8_t i = 0; i < kBitsInByte; ++i) {
            bits.push_back((info >> (kBitsInByte - i - 1)) & 1);
        }
    }
}

// Считаем биты имени файла

void GetOriginFilenameBits(std::vector<bool>& bits, const char* filename) {
    size_t i = 0;
    while (filename[i] != '\0') {
        char info = filename[i];
        for (uint8_t j = 0; j < kBitsInByte; ++j) {
            bits.push_back((info >> (kBitsInByte - j - 1)) & 1);
        }
        ++i;
    }
}

void WriteFilenameOrFileToArchive(std::fstream& archive, const std::vector<const char*>& origin, const Arguments& parse_arguments, size_t ind, bool is_file, bool is_filename) {
    std::vector<bool> object_bits, coded_object_bits;

    if (is_file) {
        std::fstream file(origin[ind], std::ios::in | std::ios::binary);
        GetOriginFileBits(object_bits, file);
    }
    if (is_filename) {
        GetOriginFilenameBits(object_bits, origin[ind]);
    }

    Hamming::HammingCode(object_bits, coded_object_bits, parse_arguments.hamming_block);

    size_t coded_object_bits_len = coded_object_bits.size();

    std::vector<bool> len_bits;
    ConvertSizeTToBits(len_bits, coded_object_bits_len);

    std::vector<bool> coded_len_bits;
    Hamming::HammingCode(len_bits, coded_len_bits, 8); // Делаем длину блока фиксированной для более удобного дальнейшего декодирования. Параметр Хэмминга влияет лишь на то, какими блоками кодировать имя файла и сам файл

    std::vector<uint8_t> coded_len_bytes, coded_object_bytes;
    ConvertBitsToBytes(coded_len_bits, coded_len_bytes);
    ConvertBitsToBytes(coded_object_bits, coded_object_bytes);
    
    for (size_t i = 0; i < coded_len_bytes.size(); ++i) {
        archive << coded_len_bytes[i];
    }
    
    for (size_t i = 0; i < coded_object_bytes.size(); ++i) {
        archive << coded_object_bytes[i];
    }
}

void WriteFilesToArchive(std::fstream& archive, const std::vector<const char*>& origin, const Arguments& parse_arguments) {
    for (size_t i = 0; i < origin.size(); ++i) {
        WriteFilenameOrFileToArchive(archive, origin, parse_arguments, i, false, true);
        WriteFilenameOrFileToArchive(archive, origin, parse_arguments, i, true, false);
    }
}

// Вспомогательные функции для нахождения закодированной, декодированной длины закодированного имени файла / закодированного файла, преобразования ее в size_t

size_t ConvertDecodedLenToSizeT(std::vector<uint8_t>& decoded_len) {
    size_t len = 0;
    for (uint8_t i = 0; i < sizeof(size_t); ++i) {
        len |= (decoded_len[i] << (sizeof(size_t) * (sizeof(size_t) - i - 1)));
    }

    return len;
}

void ConvertCodedLenBytesToBits(std::vector<uint8_t>& coded_bytes, std::vector<bool>& coded_bits) {
    for (size_t i = 0; i < coded_bytes.size(); ++i) {
        for (uint8_t j = 0; j < kBitsInByte; ++j) {
            coded_bits.push_back((coded_bytes[i]  >> (kBitsInByte - 1 - j)) & 1);
        }
    }
}

void GetCodedLen(std::fstream& archive, const std::string& archive_path, std::vector<uint8_t>& coded_len_bytes, std::vector<bool>& coded_len_bits) {
    GetNextNthBytes(archive, archive_path, kCodedLenSizeInBytes, coded_len_bytes);

    ConvertCodedLenBytesToBits(coded_len_bytes, coded_len_bits);
}

void GetDecodedLen(std::fstream& archive, size_t hamming_block, std::vector<bool>& coded_len_bits, std::vector<uint8_t>& decoded_len_bytes) {
    Hamming::HammingDecode(coded_len_bits, decoded_len_bytes, hamming_block);
}

size_t GetSizeTDecodedLen(std::fstream& archive, const std::string& archive_path, size_t hamming_block) {
    std::vector<uint8_t> coded_len_bytes;
    std::vector<bool> coded_len_bits;
    GetCodedLen(archive, archive_path, coded_len_bytes, coded_len_bits);

    std::vector<uint8_t> decoded_len;
    GetDecodedLen(archive, hamming_block, coded_len_bits, decoded_len);

    return ConvertDecodedLenToSizeT(decoded_len);
}

// Вспомогательные функции для получения закодированного / декодированного имени файла / файла, а также преобразования из вектора чаров в массив чаров
void GetCodedFilenameOrFileBits(std::vector<uint8_t>& coded_bytes, std::vector<bool>& coded_bits, size_t bits_count) {
    size_t counter = 0;

    for (size_t i = 0; i < coded_bytes.size() - 1; ++i) {
        for (uint8_t j = 0; j < kBitsInByte; ++j) {
            coded_bits.push_back((coded_bytes[i] >> (kBitsInByte - 1 - j)) & 1);
            ++counter;
        }
    }

    if (bits_count % 8 != 0) {
        for (uint8_t i = 0; i < bits_count % 8; ++i) {
            coded_bits.push_back((coded_bytes.back() >> (kBitsInByte - 1 - i)) & 1);
        }
    } else {
        for (uint8_t i = 0; i < kBitsInByte; ++i) {
            coded_bits.push_back((coded_bytes.back() >> (kBitsInByte - 1 - i)) & 1);
        }
    }
}

void GetCodedFilenameOrFile(std::fstream& archive, const std::string& archive_path, size_t len, std::vector<uint8_t>& coded_bytes, std::vector<bool>& coded_bits) {
    size_t len_in_bytes = len / 8;

    if (len % 8 != 0) {
        ++len_in_bytes;
    }

    GetNextNthBytes(archive, archive_path, len_in_bytes, coded_bytes);

    GetCodedFilenameOrFileBits(coded_bytes, coded_bits, len);
}

void GetDecodedFilenameOrFile(std::vector<bool>& coded_bits, std::vector<uint8_t>& decoded_bytes, size_t hamming_block) {
    Hamming::HammingDecode(coded_bits, decoded_bytes, hamming_block);
}

void GetPointerOnCharFromDecodedObject(std::vector<uint8_t>& object, char* ptr) {
    for (size_t i = 0; i < object.size(); ++i) {
        ptr[i] = object[i];
    }
    ptr[object.size()] = '\0';
}

// Считывание закодированной длины закодированного имени файла/ закодированного файла + пропуск закодированного имени файла/ закодированного файла

void Skip(std::fstream& archive, const std::string& archive_path, size_t hamming_block) {
    size_t len_in_bits = GetSizeTDecodedLen(archive, archive_path, hamming_block);

    size_t len_in_bytes = len_in_bits / 8;
    if (len_in_bits % 8 != 0) {
        ++len_in_bytes;
    }

    archive.seekg(len_in_bytes, std::ios_base::cur);
}

// Создать файл с именем filename и байтами из bytes

void CreateFile(const char* filename, std::vector<uint8_t>& bytes) {
    std::fstream new_file(filename, std::ios::out | std::ios::binary);

    for (size_t i = 0; i < bytes.size(); ++i) {
        new_file << bytes[i];
    }

    new_file.close();
}

// Извлечь файл с именем search или извлечь все, если extract_all_files = true

void ExtractFile(std::fstream& archive, const std::string& archive_path, const char* search, size_t hamming_block, bool extract_all_files) {
    std::vector<uint8_t> coded_filename_bytes;
    std::vector<bool> coded_filename_bits;

    std::vector<uint8_t> decoded_filename_bytes;

    std::vector<uint8_t> coded_file_bytes;
    std::vector<bool> coded_file_bits;

    std::vector<uint8_t> decoded_file_bytes;

    while (std::filesystem::file_size(std::filesystem::path(archive_path)) > archive.tellg()) {
        size_t filename_decoded_len = GetSizeTDecodedLen(archive, archive_path, hamming_block);

        GetCodedFilenameOrFile(archive, archive_path, filename_decoded_len, coded_filename_bytes, coded_filename_bits);
        GetDecodedFilenameOrFile(coded_filename_bits, decoded_filename_bytes, hamming_block);

        char* filename = new char[decoded_filename_bytes.size() + 1];
        GetPointerOnCharFromDecodedObject(decoded_filename_bytes, filename);

        if (std::strcmp(filename, search) != 0 && !extract_all_files) {
            Skip(archive, archive_path, hamming_block);

            delete[] filename;
            filename = nullptr;

            coded_filename_bytes.clear();
            coded_filename_bits.clear();
            decoded_filename_bytes.clear();

            continue;
        }

        size_t file_decoded_len = GetSizeTDecodedLen(archive, archive_path, hamming_block);

        GetCodedFilenameOrFile(archive, archive_path, file_decoded_len, coded_file_bytes, coded_file_bits);
        GetDecodedFilenameOrFile(coded_file_bits, decoded_file_bytes, hamming_block);

        CreateFile(filename, decoded_file_bytes);

        delete[] filename;
        filename = nullptr;

        coded_filename_bytes.clear();
        coded_filename_bits.clear();
        decoded_filename_bytes.clear();
        coded_file_bytes.clear();
        coded_file_bits.clear();
        decoded_file_bytes.clear();
    }
}

// Проверка на то, надо ли удалять файл

bool IsDeletableFile(const std::vector<const char*>& deletable_files, const char* filename) {
    for (size_t i = 0; i < deletable_files.size(); ++i) {
        if (!strcmp(deletable_files[i], filename)) {
            return true;
        }
    }
    return false;
}

// Запись закодированной длины закодированного имени файла/файла и закодированного имени файла/файла в вектор

void WriteInNewData(std::vector<uint8_t> new_data, const std::vector<uint8_t>& coded_len_bytes, const std::vector<uint8_t>& coded_filename_or_file_bytes) {
    for (size_t i = 0; i < coded_len_bytes.size(); ++i) {
        new_data.push_back(coded_len_bytes[i]);
    }

    for (size_t i = 0; i < coded_filename_or_file_bytes.size(); ++i) {
        new_data.push_back(coded_filename_or_file_bytes[i]);
    }
}

// ОСНОВНЫЕ ФУНКЦИИ

void Archive::CreateArchive(const Arguments& parse_arguments) {
    std::fstream new_archive(parse_arguments.archive_name, std::ios::out | std::ios::binary);

    if (new_archive.is_open()) {
        WriteFilesToArchive(new_archive, parse_arguments.initialize_file_names, parse_arguments);

        new_archive.close();
    }
}

void Archive::AppendFiles(const Arguments& parse_arguments) {
    std::fstream archive(parse_arguments.archive_name, std::ios::app | std::ios::ate | std::ios::binary);

    if (archive.is_open()) {
        WriteFilesToArchive(archive, parse_arguments.append_file_names, parse_arguments);

        archive.close();
    }
}

void Archive::ShowList(const Arguments& parse_arguments) {
    std::fstream archive(parse_arguments.archive_name, std::ios::in | std::ios::binary);

    std::vector<uint8_t> coded_filename_bytes;
    std::vector<bool> coded_filename_bits;

    std::vector<uint8_t> decoded_filename_bytes;

    if (archive.is_open()) {
        while (std::filesystem::file_size(std::filesystem::path(parse_arguments.archive_name)) > archive.tellg()) {
            size_t coded_filename_len = GetSizeTDecodedLen(archive, parse_arguments.archive_name, parse_arguments.hamming_block);

            GetCodedFilenameOrFile(archive, parse_arguments.archive_name, coded_filename_len, coded_filename_bytes, coded_filename_bits);
            GetDecodedFilenameOrFile(coded_filename_bits, decoded_filename_bytes, parse_arguments.hamming_block);

            char* filename = new char[decoded_filename_bytes.size() + 1];
            GetPointerOnCharFromDecodedObject(decoded_filename_bytes, filename);

            std::cout << filename << '\n';

            Skip(archive, parse_arguments.archive_name, parse_arguments.hamming_block);

            delete[] filename;
            filename = nullptr;

            coded_filename_bytes.clear();
            coded_filename_bits.clear();
            decoded_filename_bytes.clear();
        }
    }
}

void Archive::ConcatenateArchives(const Arguments& parse_arguments) {
    std::ifstream first_archive(parse_arguments.first_archive_for_concatenate, std::ios::in | std::ios::binary);

    std::ifstream second_archive(parse_arguments.second_archive_for_concatenate, std::ios::in | std::ios::binary);

    std::fstream result_archive(parse_arguments.archive_name, std::ios::out | std::ios::binary);

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
    std::fstream archive(parse_arguments.archive_name, std::ios::in | std::ios::binary);

    if (archive.is_open()) {
        if (parse_arguments.extract_all_files) {
            ExtractFile(archive, parse_arguments.archive_name, "", parse_arguments.hamming_block, true);
        } else {
            for (size_t i = 0; i < parse_arguments.extractable_files.size(); ++i) {
                ExtractFile(archive, parse_arguments.archive_name, parse_arguments.extractable_files[i], parse_arguments.hamming_block, false);
            }
        }
        archive.close();
    }
}


void Archive::Delete(const Arguments& parse_arguments) {
    std::fstream archive(parse_arguments.archive_name, std::ios::in | std::ios::binary);

    std::vector<uint8_t> coded_filename_bytes;
    std::vector<bool> coded_filename_bits;

    std::vector<uint8_t> decoded_filename_bytes;

    std::vector<uint8_t> coded_file_bytes;
    std::vector<bool> coded_file_bits;

    if (archive.is_open()) {
        std::vector<uint8_t> new_data;
        while (std::filesystem::file_size(std::filesystem::path(parse_arguments.archive_name)) > archive.tellg()) {

            std::vector<uint8_t> coded_filename_len_bytes;
            std::vector<bool> coded_filename_len_bits;
            GetCodedLen(archive, parse_arguments.archive_name, coded_filename_len_bytes, coded_filename_len_bits);

            std::vector<uint8_t> decoded_filename_len;
            GetDecodedLen(archive, parse_arguments.hamming_block, coded_filename_len_bits, decoded_filename_len);

            size_t coded_filename_len = ConvertDecodedLenToSizeT(decoded_filename_len);

            GetCodedFilenameOrFile(archive, parse_arguments.archive_name, coded_filename_len, coded_filename_bytes, coded_filename_bits);
            GetDecodedFilenameOrFile(coded_filename_bits, decoded_filename_bytes, parse_arguments.hamming_block);

            char* filename = new char[decoded_filename_bytes.size() + 1];
            GetPointerOnCharFromDecodedObject(decoded_filename_bytes, filename);

            if (IsDeletableFile(parse_arguments.deletable_files, filename)) {
                Skip(archive, parse_arguments.archive_name, parse_arguments.hamming_block);

                delete[] filename;
                filename = nullptr;

                coded_filename_bytes.clear();
                coded_filename_bits.clear();
                decoded_filename_bytes.clear();

                continue;
            }

            std::vector<uint8_t> coded_file_len_bytes;
            std::vector<bool> coded_file_len_bits;
            GetCodedLen(archive, parse_arguments.archive_name, coded_file_len_bytes, coded_file_len_bits);

            std::vector<uint8_t> decoded_file_len;
            GetDecodedLen(archive, parse_arguments.hamming_block, coded_file_len_bits, decoded_file_len);

            size_t coded_file_len = ConvertDecodedLenToSizeT(decoded_file_len);

            GetCodedFilenameOrFile(archive, parse_arguments.archive_name, coded_file_len, coded_file_bytes, coded_file_bits);

            WriteInNewData(new_data, coded_filename_len_bytes, coded_file_bytes);
            WriteInNewData(new_data, coded_file_len_bytes, coded_file_bytes);

            delete[] filename;
            filename = nullptr;

            coded_filename_bytes.clear();
            coded_filename_bits.clear();
            decoded_filename_bytes.clear();
            coded_file_bytes.clear();
            coded_file_bits.clear();
        }

        archive.close();

        archive.open(parse_arguments.archive_name, std::ios::out | std::ios::binary);

        for (size_t i = 0; i < new_data.size(); ++i) {
            archive << new_data[i];
        }

        archive.close();
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