#pragma once

#include <vector>
#include <cstdint>
#include <string>

class Hamming {
public:
    static size_t CalcControlBits(size_t hamming_block);
    static void HammingCode(const std::vector<bool>& default_bits, std::vector<bool>& coded_bits, size_t hamming_block);
    static void HammingDecode(std::vector<bool>& coded_bits, std::vector<bool>& decoded_bits, size_t hamming_block);
};
