#pragma once

#include <filesystem>
#include <time.h>

enum class ThreatLevel : uint8_t { NONE = 1, LOW = 2, MEDIUM = 3, HIGH = 4, VERY_HIGH = 5, VIRUS = 6 };

class File {
private:
    std::filesystem::path m_FilePath;
    std::time_t m_LastModificationData;
    uintmax_t size;
    std::string m_HashID;
    ThreatLevel m_ThreatLevel = ThreatLevel::NONE;

public:
    explicit File(const std::filesystem::path &file_path);
};
