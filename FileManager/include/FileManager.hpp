#pragma once

#include <filesystem>

class FileManager {
private:
        std::filesystem::path m_FilePath;
        size_t m_Size = 0;
        std::time_t m_LastModificationData = 0;
        std::string m_HashID;

public:
        FileManager();
        explicit FileManager(const std::filesystem::path& file_path);

        auto compute_hash() -> void;
        auto update_metadata() -> void;
        auto save_to_csv() -> void;
};