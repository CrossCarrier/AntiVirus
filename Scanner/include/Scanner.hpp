#pragma once

#include <unordered_map>
#include <utility>

#include "../../DirectoryManager/include/Directory.hpp"
#include "../../FileManager/include/File.hpp"
#include "../../RuleEngine/include/Rule.hpp"

#define PERCENTAGE_MATCH int

using SingleRuleScan_INFO = std::pair<Rule, PERCENTAGE_MATCH>;

class Scanner {
     private:
     public:
};
