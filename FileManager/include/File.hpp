#pragma once

#include <filesystem>

enum class ThreatLevel : uint8_t { NONE = 1, LOW = 2, MEDIUM = 3, HIGH = 4, VERY_HIGH = 5, VIRUS = 6 };

using FILE_PATH = std::filesystem::path;
class File {
private:
    FILE_PATH m_FilePath;
    ThreatLevel m_ThreatLevel;
    std::time_t m_LastModificationData;
    std::string m_HashID;
    uintmax_t size;

public:
    explicit File(const std::filesystem::path &file_path);

    auto update_index() -> void;
};
