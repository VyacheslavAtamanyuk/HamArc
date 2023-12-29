#ifndef LABWORK6_HAMMINGCODE_H
#define LABWORK6_HAMMINGCODE_H

#include <vector>
#include <cstdint>
#include <string>

class Hamming {
public:
    static size_t CalcControlBits(size_t hamming_block);
    static size_t HammingCode(std::vector<uint8_t>& data, size_t hamming_block, const std::string& file, uint8_t* size_container, bool is_coding_file, bool is_coding_filename, bool is_coding_size);
    static void HammingDecode(std::vector<uint8_t>& coded_data, std::vector<uint8_t>& decoded_data, size_t hamming_block);
};

#endif //LABWORK6_HAMMINGCODE_H
