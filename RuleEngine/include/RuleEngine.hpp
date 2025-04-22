#pragma once

#include <filesystem>
using RULE_CONFIG_FILE = std::filesystem::path;
class RuleEngine {
private:
    static RULE_CONFIG_FILE m_ConfigFile;
public:
    static auto set_RuleConfigFile(RULE_CONFIG_FILE p_ConfigFile) -> void;
};
