#pragma once

#include <openssl/evp.h>
#include <openssl/sha.h>

#include <boost/filesystem/path.hpp>

namespace hash_SHA256 {
auto hash_file(const boost::filesystem::path &file_path) -> std::string;
};
