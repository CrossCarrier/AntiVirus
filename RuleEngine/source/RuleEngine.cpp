#include "../include/RuleEngine.hpp"

namespace rule_engine {
    namespace config_files {
        constexpr const char *YARA_RULES_CONFIG = "../antivirus/YARA_config.json";
    }
    auto get_Rules() noexcept -> std::vector<std::string> {
        nlohmann::json rules = json_manager::read_data(rule_engine::config_files::YARA_RULES_CONFIG);
        return rules;
    }
} // namespace rule_engine
