#pragma once

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <vector>

using SCAN_RESULTS = std::vector<std::string>;
class File {
private:
    boost::filesystem::path m_FilePath;
    std::time_t m_LastModificationData;
    std::string m_HashID;
    uintmax_t m_Size;

public:
    explicit File(const boost::filesystem::path &file_path);

    [[nodiscard]] auto get_HashID() const noexcept -> std::string;
    [[nodiscard]] auto get_FilePath() const noexcept -> boost::filesystem::path;
    [[nodiscard]] auto get_LastModificationTime() const noexcept -> std::time_t;
    [[nodiscard]] auto get_Size() const noexcept -> std::uintmax_t;

    [[nodiscard]] auto set_ScanningResults(const SCAN_RESULTS *_results) const -> void;
};
