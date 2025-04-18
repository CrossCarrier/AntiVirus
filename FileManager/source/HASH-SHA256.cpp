#include "../include/HASH-SHA256.hpp"
#include <openssl/sha.h>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <stdexcept>

namespace hash_SHA256 {
    auto hash_file(const std::filesystem::path& file_path) -> std::string {
        std::ifstream file(file_path.c_str(), std::ios::binary);
        if (!file) {
            throw std::runtime_error("Failed to open file " + file_path.string());
        }

        SHA256_CTX ctx;
        SHA256_Init(&ctx);

        // Reading 4KiB of data per read and hashing it
        char data[4096];
        while (file.read(data, sizeof(data))) {
            SHA256_Update(&ctx, data, file.gcount());
        }
        if (file.gcount() > 0) {
            SHA256_Update(&ctx, data, file.gcount());
        }

        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_Final(hash, &ctx);

        std::stringstream ss;
        for (auto& byte : hash) {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
        }

        return ss.str();
    }
}