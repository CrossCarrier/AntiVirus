#pragma once

#include <boost/filesystem/path.hpp>
#include <openssl/evp.h>
#include <openssl/sha.h>

namespace hash_SHA256 {
    auto hash_file(const boost::filesystem::path &file_path) -> std::string;
};
