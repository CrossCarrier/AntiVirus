#include "../include/HASH-SHA256.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iomanip>

#define EVP_MAX_SIZE 32
#define READING_BUFFOR 2048

namespace hash_SHA256 {
    auto hash_file(const boost::filesystem::path &file_path) -> std::string {
        std::ifstream file(file_path.c_str(), std::ios::binary);
        if (!file) {
            throw std::runtime_error("Failed to open file : " + file_path.string());
        }

        auto m_Context = EVP_MD_CTX_create();
        auto m_HashingAlgorithm = EVP_get_digestbyname("sha256");
        unsigned char l_HashedFile[EVP_MAX_SIZE];

        EVP_DigestInit_ex(m_Context, m_HashingAlgorithm, NULL);

        char l_Data[READING_BUFFOR];
        while (file) {
            file.read(l_Data, READING_BUFFOR);
            if (file.gcount() > 0) {
                EVP_DigestUpdate(m_Context, l_Data, file.gcount());
            }
        }

        EVP_DigestFinal_ex(m_Context, l_HashedFile, NULL);

        std::stringstream ss;
        for (const auto &byte: l_HashedFile) {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
        }

        EVP_MD_CTX_free(m_Context);

        return ss.str();
    }
} // namespace hash_SHA256
