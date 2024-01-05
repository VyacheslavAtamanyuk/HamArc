#ifndef LABWORK6_SOMEGENERALFUNCTIONS_H
#define LABWORK6_SOMEGENERALFUNCTIONS_H

#include <fstream>
#include <cstdint>
#include <vector>

class SomeGeneralFunctions {
public:
    static void GetNextNthDecodedBytes(std::ifstream& archive, const std::string& archive_path, size_t hamming_block, size_t n, std::vector<uint8_t>& coded_bytes, std::vector<uint8_t>& decoded_bytes);
    static size_t ConvertBytesToSizeT(const std::vector<uint8_t>& bytes);
    static void GetPointerOnCharFromDecodedObject(const std::vector<uint8_t>& object, char* ptr);
    static void Skip(std::ifstream& archive, const std::string& archive_path, size_t hamming_block);
};

#endif //LABWORK6_SOMEGENERALFUNCTIONS_H
