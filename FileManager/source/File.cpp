#include "../include/File.hpp"
#include <boost/filesystem/operations.hpp>
#include <iostream>
#include "../include/HASH-SHA256.hpp"

File::File(const boost::filesystem::path &file_path) {
    if (!boost::filesystem::exists(file_path)) {
        throw std::invalid_argument("File seems to not exist!\n");
    }

    this->m_FilePath = boost::filesystem::absolute(file_path);
    this->m_ThreatLevel = ThreatLevel::NONE;
    this->m_LastModificationData = boost::filesystem::last_write_time(file_path);
    this->m_Size = boost::filesystem::file_size(file_path);

    try {
        this->m_HashID = hash_SHA256::hash_file(file_path);
    } catch (std::exception &error) {
        std::cerr << error.what() << std::endl;
    }
}

auto File::get_HashID() const noexcept -> std::string { return this->m_HashID; }
auto File::get_FilePath() const noexcept -> boost::filesystem::path { return this->m_FilePath; }
auto File::get_LastModificationTime() const noexcept -> std::time_t { return this->m_LastModificationData; }
auto File::get_Size() const noexcept -> std::uintmax_t { return this->m_Size; }


auto File::assign_threat_level(const ThreatLevel &_threat_level) -> void {
    this->m_ThreatLevel = _threat_level;
}
