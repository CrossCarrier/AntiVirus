#pragma once

#include <string>

enum class RuleType : uint8_t { STRING_MATCH = 1, REGEX_PATTERN = 2, HASH_MATCH = 3, BEHAVIOUR_PATTERN = 4 };
enum class RuleAction : uint8_t { REPORT = 1, QUARANTINE = 2, REMOVE = 3, BLOCK = 4 };
class Rule {
private:
    std::string m_Id;
    std::string m_Name;
    std::string m_Pattern;
    RuleType m_RuleType;
    RuleAction m_RuleAction;

public:
    Rule(const std::string &p_Id, const std::string &p_Name, const std::string &p_Pattern, const RuleType &p_RuleType,
         const RuleAction &p_RuleAction);

    auto match(const std::string &p_Content, const std::string &p_Hash) -> bool;
};
