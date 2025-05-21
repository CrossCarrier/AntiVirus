#pragma once

#include <filesystem>
#include <vector>

using SCAN_RESULTS = std::vector<std::string>;
namespace fs = std::filesystem;
class File {
private:
    fs::path m_FilePath;
    time_t m_LastModificationData;
    std::string m_HashID;
    uintmax_t m_Size;

public:
    explicit File(const std::filesystem::path &file_path);

    [[nodiscard]] auto get_HashID() const noexcept -> std::string;
    [[nodiscard]] auto get_FilePath() const noexcept -> fs::path;
    [[nodiscard]] auto get_LastModificationTime() const noexcept -> std::time_t;
    [[nodiscard]] auto get_Size() const noexcept -> std::uintmax_t;

    auto operator==(const File &other) -> bool {
        if (m_FilePath == other.m_FilePath && m_LastModificationData == other.m_LastModificationData &&
            m_HashID == other.m_HashID && m_Size == other.m_Size) {
            return true;
        }
        return false;
    }
};
