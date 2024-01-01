#ifndef LABWORK6_HAMMINGCODE_H
#define LABWORK6_HAMMINGCODE_H

#include <vector>
#include <cstdint>
#include <string>

class Hamming {
public:
    static size_t CalcControlBits(size_t hamming_block);
    static void HammingCode(std::vector<bool>& default_bits, std::vector<bool>& coded_bits, size_t hamming_block);
    static void HammingDecode(std::vector<bool>& coded_bits, std::vector<uint8_t>& decoded_bytes, size_t hamming_block);
};

#endif //LABWORK6_HAMMINGCODE_H
