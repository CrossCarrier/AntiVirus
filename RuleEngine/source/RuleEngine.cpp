#include "../include/RuleEngine.hpp"
#include "../../HELPERS/include/support.hpp"

namespace rule_engine {
    namespace config_files {
        constexpr const char *YARA_RULES_CONFIG = "../antivirus/AppData/YARA_config.json";
    }

    auto get_Rules() noexcept -> std::vector<std::string> {
        std::vector<std::string> result;
        auto readed_data = support::json_utils::read_data(std::move(std::filesystem::path(config_files::YARA_RULES_CONFIG)));
        std::ranges::for_each(readed_data, [&](const std::string &__val) -> void { result.emplace_back(std::move(__val)); });

        return result;
    }
} // namespace rule_engine
