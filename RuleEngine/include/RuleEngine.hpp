#pragma once

#include <boost/filesystem/path.hpp>
#include <string>
#include <vector>

class RuleEngine {
public:
    explicit RuleEngine(const boost::filesystem::path &_config_file);
    auto get_Rules() const noexcept -> const std::vector<std::string> &;

private:
    std::vector<std::string> m_Rules;
};
