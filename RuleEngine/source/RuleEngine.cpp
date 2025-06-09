#include "../include/RuleEngine.hpp"
#include "../../HELPERS/include/support.hpp"
#include "../../Logger/include/Logger.hpp"
#include "../../ERRORS_PACK/include/errors.hpp"
#include <format>

namespace rule_engine {

    namespace config_files {
        constexpr auto YARA_RULES_CONFIG = "../antivirus/AppData/YARA_config.json";
    }

    auto get_Rules() -> RULES_PACK {
        auto& logger = logger::Logger::getInstance();
        logger.info("Loading YARA rules configuration");
        logger.debug(std::format("Reading rules from config file: {}", config_files::YARA_RULES_CONFIG));

        RULES_PACK result;

        try {
            logger.debug("Attempting to parse YARA rules configuration");
            JSON data = support::json_utils::read_data((PATH(config_files::YARA_RULES_CONFIG)));

            logger.debug(std::format("Successfully loaded rules config with {} entries", data.size()));

            logger.debug("Processing rule entries");
            size_t ruleCount = 0;
            std::ranges::for_each(data, [&](const std::string &val) -> void {
                logger.debug(std::format("Adding rule: {}", val));
                result.emplace_back((val));
                ruleCount++;
            });

            logger.success(std::format("Successfully loaded {} YARA rules", ruleCount));

        } catch (const PathNotFound& e) {
            logger.error(std::format("YARA rules config file not found: {}", e.what()));
            throw;
        } catch (const StreamOpeningError& e) {
            logger.error(std::format("Failed to open YARA rules config file: {}", e.what()));
            throw;
        } catch (const nlohmann::json::exception& e) {
            logger.error(std::format("JSON parsing error in YARA rules config: {}", e.what()));
            throw;
        } catch (const std::exception& e) {
            logger.error(std::format("Error loading YARA rules: {}", e.what()));
            throw;
        }

        logger.info(std::format("Returning {} YARA rules", result.size()));
        return result;
    }

} // namespace rule_engine