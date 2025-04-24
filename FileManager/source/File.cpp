#include "../include/File.hpp"
#include <boost/filesystem/operations.hpp>
#include <iostream>
#include "../include/HASH-SHA256.hpp"


File::File(const boost::filesystem::path &file_path) {
    if (!boost::filesystem::exists(file_path)) {
        throw std::invalid_argument("File seems to not exist!\n");
    }

    this->m_FilePath = std::move(file_path);

    /*SCAN IS NEEDED TO PERFORM UPDATING THREAT LEVEL*/
    /*DEFAULT THREAT LEVEL IS NONE*/
    this->m_ThreatLevel = ThreatLevel::NONE;

    try {
        this->m_LastModificationData = boost::filesystem::last_write_time(file_path);
        this->m_HashID = hash_SHA256::hash_file(file_path);
        this->m_Size = boost::filesystem::file_size(file_path);
    } catch (std::exception &error) {
        std::cerr << error.what() << std::endl;
    }
}

auto File::assign_threat_level(const ThreatLevel &_threat_level) -> void {
    this->m_ThreatLevel = std::move(_threat_level);
}
