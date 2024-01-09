#pragma once

#include <cstdint>
#include <vector>
#include <fstream>

struct IterationTools {
    IterationTools(size_t block);

    size_t bytes_count = 0;
    size_t bits_count = 0;

    size_t current_symbols_count = 0;
    size_t remainder = 0;

    std::vector<uint8_t> buffer_bytes;
    std::vector<bool> buffer_bits;

    std::vector<bool> remainder_in_bytes;
    std::vector<bool> remainder_in_block;

    size_t i = 0;

    bool is_it_end = false;
};

class DecodeAndEncode {
public:
    void UpdateTools(size_t default_bits_count_value, IterationTools& tools);
    void SetCurrentRemainderInBytes(const std::vector<bool>& bits, std::vector<bool>& remainder_in_block);

    void UpdateRemainderInDecodedBytes(std::vector<bool>& remainder_in_bytes);
    void UpdateRemainderInEncodedBytes(std::ofstream& archive, std::vector<bool>& remainder_in_bytes, bool is_it_end);

    void ConvertBufferBytesToBits(IterationTools& tools, bool is_it_decode_process);

    void DecodeBlock(std::vector<bool>& decoded_bits, size_t hamming_block, IterationTools& decode_tools);
    void EncodeBlock(std::vector<bool>& encoded_bits, size_t hamming_block, IterationTools& encode_tools);

    void AddDecodedBitsToString(std::vector<bool>& decoded_bits, std::string& string_with_decoded_bytes, std::vector<bool>& remainder_in_bytes);
    void AddDecodedBitsToFile(std::vector<bool>& decoded_bits, std::ofstream& file, std::vector<bool>& remainder_in_bytes);
    void AddDecodedBitsToSizeT(std::vector<bool>& decoded_bits, size_t& object_len, std::vector<bool>& remainder_in_bytes, uint8_t& insertable_idx);

    void DecodeBytesAndPutToString(char info, size_t hamming_block, std::string& string_with_decoded_bytes, IterationTools& decode_tools);
    void DecodeBytesAndPutToFile(char info, size_t hamming_block, std::ofstream& file, IterationTools& decode_tools);
    void DecodeBytesAndPutToSizeT(char info, size_t hamming_block, size_t& object_len, IterationTools& decode_tools, uint8_t& insertable_idx);

    void AddEncodedBitsToArchive(const std::vector<bool>& encoded_bits, std::ofstream& archive, std::vector<bool>& remainder_in_bytes, bool is_it_end);
    void EncodeBytesAndPutToArchive(char info, size_t hamming_block, std::ofstream& archive, IterationTools& encode_tools);
};
