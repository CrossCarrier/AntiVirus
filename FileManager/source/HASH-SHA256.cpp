#include "../include/HASH-SHA256.hpp"
#include "../../Logger/include/Logger.hpp"
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <format>

#include <openssl/evp.h>
#include <openssl/sha.h>

namespace {
    constexpr int EVP_MAX_SIZE = 32;
    constexpr int READING_BUFFER = 2048;
}

namespace hash_SHA256 {
    auto hash_file(const std::filesystem::path &file_path) -> std::string {
        auto& logger = logger::Logger::getInstance();
        logger.debug(std::format("Starting SHA-256 hash calculation for: {}", file_path.string()));

        std::ifstream file(file_path.c_str(), std::ios::binary);
        if (!file) {
            logger.error(std::format("Failed to open file for hashing: {}", file_path.string()));
            throw std::runtime_error("Failed to open file : " + file_path.string());
        }
        logger.debug(std::format("File opened successfully: {}", file_path.string()));

        logger.debug("Initializing EVP context for SHA-256 hashing");
        auto m_Context = EVP_MD_CTX_create();
        if (!m_Context) {
            logger.error("Failed to create EVP context for hashing");
            throw std::runtime_error("Failed to create EVP context");
        }

        auto m_HashingAlgorithm = EVP_get_digestbyname("sha256");
        if (!m_HashingAlgorithm) {
            logger.error("Failed to get SHA-256 algorithm");
            EVP_MD_CTX_free(m_Context);
            throw std::runtime_error("Failed to get SHA-256 algorithm");
        }

        unsigned char l_HashedFile[EVP_MAX_SIZE];

        if (!EVP_DigestInit_ex(m_Context, m_HashingAlgorithm, nullptr)) {
            logger.error("Failed to initialize digest");
            EVP_MD_CTX_free(m_Context);
            throw std::runtime_error("Failed to initialize digest");
        }

        logger.debug("Beginning file read operations for hashing");
        char l_Data[READING_BUFFER];
        size_t total_bytes_read = 0;
        while (file) {
            file.read(l_Data, READING_BUFFER);
            auto bytes_read = file.gcount();

            if (bytes_read > 0) {
                total_bytes_read += bytes_read;
                if (!EVP_DigestUpdate(m_Context, l_Data, bytes_read)) {
                    logger.error("Failed to update digest");
                    EVP_MD_CTX_free(m_Context);
                    throw std::runtime_error("Failed to update digest");
                }
            }
        }
        logger.debug(std::format("Completed reading file: {} bytes processed", total_bytes_read));

        logger.debug("Finalizing hash calculation");
        if (!EVP_DigestFinal_ex(m_Context, l_HashedFile, nullptr)) {
            logger.error("Failed to finalize digest");
            EVP_MD_CTX_free(m_Context);
            throw std::runtime_error("Failed to finalize digest");
        }

        std::stringstream ss;
        for (int i = 0; i < EVP_MAX_SIZE; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(l_HashedFile[i]);
        }
        std::string hash_result = ss.str();

        logger.debug("Cleaning up EVP context");
        EVP_MD_CTX_free(m_Context);

        // Log a truncated version of the hash to avoid excessive log size
        logger.debug(std::format("SHA-256 hash calculation completed for {}: {}...",
            file_path.string(), hash_result.substr(0, 16) + "..."));

        return hash_result;
    }
} // namespace hash_SHA256