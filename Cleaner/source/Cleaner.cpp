#include "../include/Cleaner.hpp"
#include "../../Logger/include/Logger.hpp"
#include <string>
#include <filesystem>
#include "../../FileManager/include/FileManager.hpp"
#include "../../FileManager/include/IndexManager.hpp"
#include <format>

namespace cleaner
{
    auto removeInfected(const std::string& output_json_path) -> void
    {
        auto& logger = logger::Logger::getInstance();

        logger.logCleaningStart();
        logger.debug(std::format("Reading infection report from: {}", output_json_path));

        nlohmann::json infected;
        try
        {
            infected = support::json_utils::read_data(output_json_path);
            logger.debug(std::format("Successfully loaded infection data from {}", output_json_path));
        }
        catch (const std::exception &e)
        {
            logger.error(std::format("Failed to read infection data from {}: {}", output_json_path, e.what()));
            throw;
        }

        logger::CleaningResult clean_result;

        for (auto& [filePath, detectionInfo] : infected.items())
        {
            if (!detectionInfo.contains("detection status") || detectionInfo["detection status"].get<std::string>() != "detected")
            {
                logger.debug(std::format("Skipping file {} - not marked as infected", filePath));
                continue;
            }

            logger.logFileCleaningAttempt(filePath);

            try
            {
                logger.debug(std::format("Updating index for file removal: {}", filePath));
                index_manager::updateAfterRemoval(filePath);

                if (!std::filesystem::exists(filePath))
                {
                    logger.warn(std::format("File {} no longer exists, skipping removal", filePath));
                    continue;
                }

                auto file_size = std::filesystem::file_size(filePath);
                bool removed = std::filesystem::remove(filePath);

                if (removed)
                {
                    logger.logFileCleaningSuccess(filePath);
                    logger.debug(std::format("Removed infected file {} ({} bytes)", filePath, file_size));
                    clean_result.files_cleaned++;
                }
                else
                {
                    logger.logFileCleaningFailed(filePath, "File removal returned false");
                    clean_result.files_failed++;
                }

            }
            catch (const std::filesystem::filesystem_error& fs_error)
            {
                logger.logFileCleaningFailed(filePath, std::format("Filesystem error: {}", fs_error.what()));
                clean_result.files_failed++;

            }
            catch (const std::exception &e)
            {
                logger.logFileCleaningFailed(filePath, std::format("General error: {}", e.what()));
                clean_result.files_failed++;
            }
        }

        clean_result.success = true;
        logger.logCleaningResult(clean_result);

        if (clean_result.files_cleaned == 0 && clean_result.files_failed == 0)
        {
            logger.info("No infected files found to clean");
        }
        else
        {
            logger.info(std::format("Cleaning operation summary: {} files cleaned, {} failed",
                        clean_result.files_cleaned, clean_result.files_failed));
        }
    }
}

