#ifndef LABWORK6_SOMEFUNCTIONSFOREXTRACTSHOWDELETE_H
#define LABWORK6_SOMEFUNCTIONSFOREXTRACTSHOWDELETE_H

#include <cstdint>
#include <vector>
#include <fstream>

struct VectorAndPointerTools {
    std::vector<uint8_t> coded_len_bytes;
    std::vector<uint8_t> decoded_len_bytes;

    std::vector<uint8_t> coded_filename_bytes;
    std::vector<uint8_t> decoded_filename_bytes;

    std::vector<uint8_t> coded_file_bytes;
    std::vector<uint8_t> decoded_file_bytes;

    char* origin_filename = nullptr;

    void clear();
};

class SomeFunctionsForExtractShowDelete {
public:
    static void CreateFile(const char* filename, const std::vector<uint8_t>& bytes);
    static bool IsExtractableFile(const std::vector<const char*>& extractable_files, const char* filename);
    static bool IsDeletableFile(const std::vector<const char*>& deletable_files, const char* filename);
    static void WriteInNewData(std::vector<uint8_t>& new_data, const std::vector<uint8_t>& coded_len_bytes, const std::vector<uint8_t>& coded_filename_or_file_bytes);
    static void EitherExtractOrShowOrDeleteFile(std::ifstream& archive, const std::string& archive_path, size_t hamming_block, bool is_delete, bool is_show, bool is_extract,
                                                bool extract_all_files, const std::vector<const char*>& deletable_files, const std::vector<const char*>& extractable_files);
};

#endif //LABWORK6_SOMEFUNCTIONSFOREXTRACTSHOWDELETE_H
