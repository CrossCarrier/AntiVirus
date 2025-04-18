#include "FileManager/include/HASH-SHA256.hpp"
#include <filesystem>
#include <iostream>

int main() {
    std::filesystem::path virus("virus.txt");
    std::filesystem::path virus2("virus2.txt");

    std::cout << "Virus file path -> " << absolute(virus) << std::endl;

    std::cout << "Virus hash -> " << hash_SHA256::hash_file(virus) << std::endl;
    std::cout << "Virus2 hash -> " << hash_SHA256::hash_file(virus2) << std::endl;

    return 0;
}