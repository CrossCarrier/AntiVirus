#include "../include/HASH-SHA256.hpp"
#include <openssl/sha.h>
#include <fstream>

namespace hash_SHA256 {
    auto hash_file(const std::filesystem::path& file_path) -> std::string {
        std::ifstream file(file_path.c_str(), std::ios::binary);

        SHA256_CTX ctx;
        SHA256_Init(&ctx);
        SHA256_Update(&ctx, )
    }
}