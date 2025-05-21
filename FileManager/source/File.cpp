#include "../include/File.hpp"
#include "../include/HASH-SHA256.hpp"
#include <boost/filesystem/operations.hpp>
#include <chrono>
#include <expected>
#include <filesystem>

File::File(const std::filesystem::path &file_path) {
    if (!fs::exists(file_path)) {
        throw std::invalid_argument("Invalid file path!\n");
    }

    m_FilePath = fs::absolute(file_path);

    auto file_modification_time = fs::last_write_time(m_FilePath);
    const auto system_time = std::chrono::file_clock::to_sys(file_modification_time);
    auto converted_system_time = std::chrono::time_point_cast<std::chrono::system_clock::duration>(system_time);

    m_LastModificationData = std::chrono::system_clock::to_time_t(converted_system_time);

    std::expected<std::string, std::runtime_error> hash = hash_SHA256::hash_file(m_FilePath);
    if (hash.has_value()) {
        m_HashID = std::move(hash.value());
    } else {
        throw hash.error();
    }
}

auto File::get_HashID() const noexcept -> std::string {
    return this->m_HashID;
}
auto File::get_LastModificationTime() const noexcept -> std::time_t {
    return this->m_LastModificationData;
}
auto File::get_Size() const noexcept -> std::uintmax_t {
    return this->m_Size;
}
