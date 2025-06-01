#include "../include/Cleaner.hpp"
#include <string>
#include <filesystem>
#include "../../FileManager/include/FileManager.hpp"

namespace cleaner {
    auto removeInfected(const std::string& output_json_path) -> void {
        nlohmann::json infected;
        try {
            infected = support::json_utils::read_data(output_json_path);
        } catch (const std::exception &_) {
            throw;
        }

        for (auto& [filePath, detectionInfo] : infected.items()) {
            if (!detectionInfo.contains("detection status") || detectionInfo["detection status"] != "detected") {
                continue;
            }
            try {
                std::filesystem::remove(filePath);
            } catch (const std::exception &_) {
                throw;
            }
        }

    }
}
