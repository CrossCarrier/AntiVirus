#pragma once

#include <filesystem>
#include <time.h>

enum class ThreatLevel : uint8_t { NONE = 1, LOW = 2, MEDIUM = 3, HIGH = 4, VERY_HIGH = 5, VIRUS = 6 };

class File {
private:
    std::filesystem::path m_FilePath;
    std::time_t m_LastModificationData = 0;
    std::string m_HashID;
    ThreatLevel m_ThreatLevel = ThreatLevel::NONE;

public:
    explicit File(const std::filesystem::path &file_path);

    [[nodiscard]] auto get_ThreatLevel() const -> ThreatLevel;

    auto update_metadata() -> void;
    auto save_to_csv() -> void;
};
