#include "../include/RuleEngine.hpp"
#include "../../HELPERS/include/support.hpp"

namespace rule_engine {

    namespace config_files {
        constexpr const char *YARA_RULES_CONFIG = "../antivirus/AppData/YARA_config.json";
    }

    auto get_Rules() -> RULES_PACK {
        RULES_PACK result;

        try {

            JSON data = support::json_utils::read_data((PATH(config_files::YARA_RULES_CONFIG)));
            std::ranges::for_each(data, [&](const std::string &val) -> void { result.emplace_back((val)); });

        } catch (std::exception& _) {
            throw;
        }

        return result;
    }

} // namespace rule_engine
