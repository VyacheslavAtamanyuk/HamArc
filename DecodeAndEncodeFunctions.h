#ifndef LABWORK6_DECODEANDENCODEFUNCTIONS_H
#define LABWORK6_DECODEANDENCODEFUNCTIONS_H

#include <cstdint>
#include <vector>
#include <fstream>

struct IterationTools {
    size_t bytes_count = 0;
    size_t bits_count = 0;

    size_t current_symbols_count = 0;
    size_t remainder = 0;

    std::vector<uint8_t> buffer_bytes;
    std::vector<bool> buffer_bits;

    std::vector<bool> remainder_in_bits_when_convert_to_bytes;
    std::vector<bool> remainder_in_bits_when_take_block;

    size_t i = 0;

    bool is_it_end = 0;
};

class DecodeAndEncodeFunctions {
public:
    static void IteratingOverDefaultSymbols(char info, size_t hamming_block, std::fstream& archive, IterationTools& encode_tools);
    static void ConvertCodedBitsToCodedBytes(const std::vector<bool>& coded_bits, std::vector<uint8_t>& coded_bytes, std::vector<bool>& remainder, bool is_it_end_of_the_file);
    static void ConvertDefaultBytesToDefaultBits(const std::vector<uint8_t>& bytes, std::vector<bool>& bits, std::vector<bool>& remainder_in_bits_when_take_block, size_t block_for_decode, bool is_remainder_here);
    static void IterationOverCodedSymbols(char info, size_t hamming_block, std::vector<uint8_t>& decoded_bytes, IterationTools& decode_tools);
    static void ConvertBytesToBits(const std::vector<uint8_t>& bytes, std::vector<bool>& bits, std::vector<bool>& remainder_in_bits_when_take_block, size_t block_for_decode, bool is_remainder_here, bool is_it_end);
    static void ConvertDecodedBitsToDecodedBytes(std::vector<bool>& decoded_bits, std::vector<uint8_t>& decoded_bytes, std::vector<bool>& remainder);
};

#endif //LABWORK6_DECODEANDENCODEFUNCTIONS_H
