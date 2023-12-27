#ifndef LABWORK6_HAMMINGCODE_H
#define LABWORK6_HAMMINGCODE_H

#include <vector>
#include <cstdint>
#include <string>

class Hamming {
public:
    static size_t CalcControlBits(size_t hemming_block);
    static size_t HammingCode(std::vector<uint8_t>& data, size_t hemming_block, const std::string& file, bool create_file);
    static std::vector<uint8_t> HammingDecode(std::vector<uint8_t>& data, size_t hemming_block);
};

#endif //LABWORK6_HAMMINGCODE_H
