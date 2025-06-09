#include "../include/Cleaner.hpp"
#include <string>
#include <filesystem>
#include "../../FileManager/include/FileManager.hpp"
#include "../../FileManager/include/IndexManager.hpp"

namespace cleaner {
    auto removeInfected(const std::string& output_json_path) -> void {
        nlohmann::json infected;
        try {
            infected = support::json_utils::read_data(output_json_path);
        } catch (const std::exception &_) {
            throw;
        }

        for (auto& [filePath, detectionInfo] : infected.items()) {
            if (!detectionInfo.contains("detection status") || detectionInfo["detection status"].get<std::string>() != "detected") {
                continue;
            }
            try {
                index_manager::updateAfterRemoval(filePath);
                std::filesystem::remove(filePath);
            } catch (const std::exception &_) {
                // Logging error logic
                throw;
            }
        }

    }
}
