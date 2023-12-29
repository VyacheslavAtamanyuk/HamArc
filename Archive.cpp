#include "Archive.h"
#include "Parser.h"
#include "HammingCode.h"

#include <cstring>
#include <fstream>
#include <vector>
#include <iostream>

const uint8_t kBitsInByte = 8;

// ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ

// Берем следующие n байт в файле

void GetNextNthBytes(std::fstream& archive, size_t n, std::vector<uint8_t>& object) {
    for (size_t i = 0; i < n; ++i) {
        char info = archive.get();
        object.push_back(info);
    }
}

// Вычисляем длину в байтах у закодированной длины закодированного имени файла/файла (исходная длина равна 8 байт)

size_t CalcCodedObjectLen(size_t hamming_block) {
    size_t bits = 0;
    for (uint8_t i = 0; i < (kBitsInByte * kBitsInByte) / hamming_block; ++i) {
        bits += hamming_block + Hamming::CalcControlBits(hamming_block) + 1;
    }

    uint8_t remainder = (kBitsInByte * kBitsInByte) % hamming_block;
    if (remainder != 0) {
        bits += remainder + Hamming::CalcControlBits(remainder) + 1;
    }

    uint8_t bytes = bits / 8;
    if (bits % 8 != 0) {
        ++bytes;
    }

    return bytes;
}


// Вспомогательные функции для записи файлов в архив

void PutLenToContainer(uint8_t* container, size_t len) {
    for (uint8_t i = 0; i < kBitsInByte; ++i) {
        container[i] = ((len >> (kBitsInByte * (kBitsInByte - 1 - i))) & 0xFF);
    }
}

void WriteFilenameOrFileToArchive(std::fstream& archive, const std::vector<const char*>& origin, const Arguments& parse_arguments, size_t ind, bool is_file, bool is_filename) {
    std::vector<uint8_t> hamming_code_object;
    uint8_t container_for_hamming_code_object_len[8];
    std::vector<uint8_t> hamming_code_container;

    size_t hamming_code_object_len = Hamming::HammingCode(hamming_code_object, parse_arguments.hamming_block, origin[ind], container_for_hamming_code_object_len, is_file, is_filename, false);
    PutLenToContainer(container_for_hamming_code_object_len, hamming_code_object_len);

    size_t hamming_code_container_len = Hamming::HammingCode(hamming_code_container, parse_arguments.hamming_block, origin[ind], container_for_hamming_code_object_len, false, false, true);

    for (size_t j = 0; j < hamming_code_container_len; ++j) {
        archive << hamming_code_container[j];
    }

    for (size_t j = 0; j < hamming_code_object_len; ++j) {
        archive << hamming_code_object[j];
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
    for (uint8_t i = 0; i < kBitsInByte; ++i) {
        len |= (decoded_len[i] << (kBitsInByte * (kBitsInByte - i - 1)));
    }

    return len;
}

void GetCodedFilenameOrFileLen(std::fstream& archive, size_t hamming_block, std::vector<uint8_t>& coded_object_len) {
    size_t distance = CalcCodedObjectLen(hamming_block);

    GetNextNthBytes(archive, distance, coded_object_len);
}

void GetDecodedFilenameOrFileLen(std::fstream& archive, size_t hamming_block, std::vector<uint8_t>& coded_filename_or_file_len, std::vector<uint8_t>& decoded_filename_or_file_len) {
    size_t distance = CalcCodedObjectLen(hamming_block);

    Hamming::HammingDecode(coded_filename_or_file_len, decoded_filename_or_file_len, hamming_block);
}

size_t GetSizeTDecodedLen(std::fstream& archive, size_t hamming_block) {
    std::vector<uint8_t> coded_object_len;
    GetCodedFilenameOrFileLen(archive, hamming_block, coded_object_len);

    std::vector<uint8_t> decoded_object_len;
    GetDecodedFilenameOrFileLen(archive, hamming_block, coded_object_len, decoded_object_len);

    return ConvertDecodedLenToSizeT(decoded_object_len);
}

// Вспомогательные функции для получения закодированного / декодированного имени файла / файла, а также преобразования из вектора чаров в массив чаров

void GetCodedFilenameOrFile(std::fstream& archive, size_t len, std::vector<uint8_t>& coded_smth) {
    for (size_t i = 0; i < len; ++i) {
        char info = archive.get();
        coded_smth.push_back(info);
    }
}

void GetDecodedFilenameOrFile(std::vector<uint8_t>& coded_smth, std::vector<uint8_t>& decoded_smth, size_t hamming_block) {
    Hamming::HammingDecode(coded_smth, decoded_smth, hamming_block);
}

void GetPointerOnCharFromDecodedObject(std::vector<uint8_t>& object, char* ptr) {
    for (size_t i = 0; i < object.size(); ++i) {
        ptr[i] = object[i];
    }
}

// Считывание закодированной длины закодированного имени файла/ закодированного файла + пропуск закодированного имени файла/ закодированного файла

void Skip(std::fstream& archive, size_t hamming_block) {
    size_t len = GetSizeTDecodedLen(archive, hamming_block);
    archive.seekg(len, std::ios_base::cur);
}

// Создать файл с именем filename и байтами из bytes

void CreateFile(const char* filename, std::vector<uint8_t>& bytes) {
    std::fstream new_file(filename, std::ios::out);

    for (size_t i = 0; i < bytes.size(); ++i) {
        new_file << bytes[i];
    }

    new_file.close();
}

// Извлечь файл с именем search или извлечь все, если extract_all_files = true

void ExtractFile(std::fstream& archive, const char* search, size_t hamming_block, bool extract_all_files) {
    std::vector<uint8_t> coded_filename;
    std::vector<uint8_t> decoded_filename;

    std::vector<uint8_t> coded_file;
    std::vector<uint8_t> decoded_file;

    while (!archive.eof()) {
        size_t decoded_filename_len = GetSizeTDecodedLen(archive, hamming_block);

        GetCodedFilenameOrFile(archive, decoded_filename_len, coded_filename);
        GetDecodedFilenameOrFile(coded_filename, decoded_filename, hamming_block);

        char* char_ptr_decoded_filename = new char[decoded_filename.size()];
        GetPointerOnCharFromDecodedObject(decoded_filename, char_ptr_decoded_filename);

        if (std::strcmp(char_ptr_decoded_filename, search) != 0 && !extract_all_files) {
            Skip(archive, hamming_block);

            delete[] char_ptr_decoded_filename;
            char_ptr_decoded_filename = nullptr;

            coded_filename.clear();
            decoded_filename.clear();

            continue;
        }

        size_t decoded_file_len = GetSizeTDecodedLen(archive, hamming_block);

        GetCodedFilenameOrFile(archive, decoded_file_len, coded_file);
        GetDecodedFilenameOrFile(coded_file, decoded_file, hamming_block);

        CreateFile(char_ptr_decoded_filename, decoded_file);

        delete[] char_ptr_decoded_filename;
        char_ptr_decoded_filename = nullptr;

        coded_filename.clear();
        decoded_filename.clear();
        coded_file.clear();
        decoded_file.clear();

        if (!extract_all_files) {
            break;
        }
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

void WriteInNewData(std::vector<uint8_t> new_data, const std::vector<uint8_t>& coded_len, const std::vector<uint8_t>& coded_filename_or_file) {
    for (size_t i = 0; i < coded_len.size(); ++i) {
        new_data.push_back(coded_len[i]);
    }

    for (size_t i = 0; i < coded_filename_or_file.size(); ++i) {
        new_data.push_back(coded_filename_or_file[i]);
    }
}

// ОСНОВНЫЕ ФУНКЦИИ

void Archive::CreateArchive(const Arguments& parse_arguments) {
    std::fstream new_archive(parse_arguments.archive_name, std::ios::out);

    if (new_archive.is_open()) {
        WriteFilesToArchive(new_archive, parse_arguments.initialize_file_names, parse_arguments);

        new_archive.close();
    }
}

void Archive::AppendFiles(const Arguments& parse_arguments) {
    std::fstream archive(parse_arguments.archive_name, std::ios::app | std::ios::ate);

    if (archive.is_open()) {
        WriteFilesToArchive(archive, parse_arguments.append_file_names, parse_arguments);

        archive.close();
    }
}

void Archive::ShowList(const Arguments& parse_arguments) {
    std::fstream archive(parse_arguments.archive_name, std::ios::in);

    std::vector<uint8_t> coded_filename;
    std::vector<uint8_t> decoded_filename;

    if (archive.is_open()) {
        while (!archive.eof()) {
            size_t decoded_filename_len = GetSizeTDecodedLen(archive, parse_arguments.hamming_block);

            GetCodedFilenameOrFile(archive, decoded_filename_len, coded_filename);
            GetDecodedFilenameOrFile(coded_filename, decoded_filename, parse_arguments.hamming_block);

            char* char_ptr_decoded_filename = new char[decoded_filename.size()];
            GetPointerOnCharFromDecodedObject(decoded_filename, char_ptr_decoded_filename);

            std::cout << char_ptr_decoded_filename << '\n';

            Skip(archive, parse_arguments.hamming_block);

            delete[] char_ptr_decoded_filename;
            char_ptr_decoded_filename = nullptr;

            coded_filename.clear();
            decoded_filename.clear();
        }

        archive.close();
    }
}

void Archive::ConcatenateArchives(const Arguments& parse_arguments) {
    std::fstream first_archive(parse_arguments.first_archive_for_concatenate, std::ios::in);

    std::fstream second_archive(parse_arguments.second_archive_for_concatenate, std::ios::in);

    std::fstream result_archive(parse_arguments.archive_name, std::ios::out);

    if (first_archive.is_open() && second_archive.is_open() && result_archive.is_open()) {
        while (!first_archive.eof()) {
            char info = first_archive.get();
            result_archive << info;
        }

        while (!second_archive.eof()) {
            char info = second_archive.get();
            result_archive << info;
        }

        first_archive.close();
        second_archive.close();
        result_archive.close();
    }
}

void Archive::Extract(const Arguments& parse_arguments) {
    std::fstream archive(parse_arguments.archive_name, std::ios::in);

    if (archive.is_open()) {
        if (parse_arguments.extract_all_files) {
            ExtractFile(archive, "", parse_arguments.hamming_block, true);
        } else {
            for (size_t i = 0; i < parse_arguments.extractable_files.size(); ++i) {
                ExtractFile(archive, parse_arguments.extractable_files[i], parse_arguments.hamming_block, false);
            }
        }

        archive.close();
    }
}

void Archive::Delete(const Arguments& parse_arguments) {
    std::fstream archive(parse_arguments.archive_name, std::ios::in);

    std::vector<uint8_t> coded_filename;
    std::vector<uint8_t> decoded_filename;

    std::vector<uint8_t> coded_file;

    if (archive.is_open()) {
        std::vector<uint8_t> new_data;
        while (!archive.eof()) {
            std::vector<uint8_t> coded_filename_len;
            GetCodedFilenameOrFileLen(archive, parse_arguments.hamming_block, coded_filename_len);

            std::vector<uint8_t> decoded_filename_len;
            GetDecodedFilenameOrFileLen(archive, parse_arguments.hamming_block, coded_filename_len, decoded_filename_len);

            size_t size_t_decoded_filename_len = ConvertDecodedLenToSizeT(decoded_filename_len);

            GetCodedFilenameOrFile(archive, size_t_decoded_filename_len, coded_filename);
            GetDecodedFilenameOrFile(coded_filename, decoded_filename, parse_arguments.hamming_block);

            char* filename = new char[decoded_filename.size()];
            GetPointerOnCharFromDecodedObject(decoded_filename, filename);

            if (IsDeletableFile(parse_arguments.deletable_files, filename)) {
                delete[] filename;
                filename = nullptr;

                coded_filename.clear();
                decoded_filename.clear();

                continue;
            }

            std::vector<uint8_t> coded_file_len;
            GetCodedFilenameOrFileLen(archive, parse_arguments.hamming_block, coded_file_len);

            std::vector<uint8_t> decoded_file_len;
            GetDecodedFilenameOrFileLen(archive, parse_arguments.hamming_block, coded_file_len, decoded_file_len);

            size_t size_t_decoded_file_len = ConvertDecodedLenToSizeT(decoded_file_len);

            GetCodedFilenameOrFile(archive, size_t_decoded_file_len, coded_file);

            WriteInNewData(new_data, coded_filename_len, coded_filename);
            WriteInNewData(new_data, coded_file_len, coded_file);

            delete[] filename;
            filename = nullptr;

            coded_filename.clear();
            decoded_filename.clear();
            coded_file.clear();
        }

        archive.close();

        archive.open(parse_arguments.archive_name, std::ios::out);

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