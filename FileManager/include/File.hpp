#pragma once

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include "../../RuleEngine/include/Rule.hpp"

enum class ThreatLevel : uint8_t {
    NONE      = 1,
    LOW       = 2,
    MEDIUM    = 3,
    HIGH      = 4,
    VERY_HIGH = 5,
    VIRUS     = 6
};

class File {
private:
    boost::filesystem::path m_FilePath;
    ThreatLevel m_ThreatLevel;
    std::time_t m_LastModificationData;
    std::string m_HashID;
    uintmax_t m_Size;

    RuleAction m_Action = RuleAction::NONE;

public:
    explicit File(const boost::filesystem::path &file_path);

    [[nodiscard]] auto get_HashID() const noexcept -> std::string;
    [[nodiscard]] auto get_FilePath() const noexcept -> boost::filesystem::path;
    [[nodiscard]] auto get_LastModificationTime() const noexcept -> std::time_t;
    [[nodiscard]] auto get_Size() const noexcept -> std::uintmax_t;

    auto assign_threat_level(const ThreatLevel &_threat_level) -> void;
    auto apply_rule() -> void;
};
