#include "../include/support.hpp"
#include "../include/Constants.hpp"
#include "../../ERRORS_PACK/include/errors.hpp"
#include "../../HELPERS/include/Types.hpp"
#include "../../Logger/include/Logger.hpp"
#include <fstream>
#include <unordered_set>
#include <format>

namespace {
    using namespace types::filesystem_types;

    auto is_file_accessible(const PATH &file_path) -> bool {
        auto& logger = logger::Logger::getInstance();
        logger.debug(std::format("Checking if file is accessible: {}", file_path.string()));

        std::error_code ec;

        if (!std::filesystem::exists(file_path, ec) || ec) {
            logger.debug(std::format("File does not exist or access error: {}", file_path.string()));
            if (ec) {
                logger.debug(std::format("Error code: {}", ec.message()));
            }
            return false;
        }

        if (!std::filesystem::is_regular_file(file_path, ec) || ec) {
            logger.debug(std::format("Path is not a regular file: {}", file_path.string()));
            if (ec) {
                logger.debug(std::format("Error code: {}", ec.message()));
            }
            return false;
        }

        logger.debug(std::format("File is accessible: {}", file_path.string()));
        return true;
    }
} // namespace

namespace support {
    namespace filesystem_utils {
        auto load_from_directory(const PATH &path) -> PATHS_CONTAINER {
            auto& logger = logger::Logger::getInstance();
            logger.info(std::format("Loading files from directory: {}", path.string()));

            using DIR_ENTRY = std::filesystem::directory_entry;
            PATHS_CONTAINER fetched_files;

            try {
                logger.debug("Starting directory iteration with permission denied skipping");
                DIRECTORY_ITER_R dir_iter(path, ITER_OPTIONS::skip_permission_denied);

                size_t fileCount = 0;
                std::ranges::for_each(dir_iter, [&](const DIR_ENTRY &entry) -> void {
                    if (entry.is_regular_file()) {
                        logger.debug(std::format("Adding file to result set: {}", entry.path().string()));
                        fetched_files.push_back(entry);
                        fileCount++;
                    }
                });

                logger.success(std::format("Successfully loaded {} files from directory: {}", fileCount, path.string()));
            } catch (const std::filesystem::filesystem_error& e) {
                logger.error(std::format("Filesystem error while loading from directory {}: {}", path.string(), e.what()));
                throw;
            } catch (const std::exception& e) {
                logger.error(std::format("Standard exception while loading from directory {}: {}", path.string(), e.what()));
                throw;
            }

            return fetched_files;
        }

        auto load_files_from_system() -> PATHS_CONTAINER {
            auto& logger = logger::Logger::getInstance();
            logger.info("Loading files from system directories");

            PATHS_CONTAINER data;
            std::error_code er;

            logger.debug("Preparing directory sets for system scanning");
            std::vector<std::unordered_set<std::string>> DirectoriesToBeScanned
            {
                constants::UserDirectories,
                constants::AutoStartLocations,
                constants::CriticalSystemLocalisation,
                constants::TemporaryData,
                constants::WebDirectories,
            };

            size_t totalDirectories = 0;
            size_t processedDirectories = 0;
            size_t skippedDirectories = 0;
            size_t filesAdded = 0;

            // Count total directories for progress reporting
            for (const auto& set : DirectoriesToBeScanned) {
                totalDirectories += set.size();
            }

            logger.info(std::format("Beginning scan of {} directory sets with {} total directories",
                DirectoriesToBeScanned.size(), totalDirectories));

            std::ranges::for_each(DirectoriesToBeScanned, [&](const auto& directories) -> void {
                std::ranges::for_each(directories, [&](const auto& directoryName) -> void {
                    processedDirectories++;
                    logger.debug(std::format("Processing directory {}/{}: {}",
                        processedDirectories, totalDirectories, directoryName));

                    if (!std::filesystem::exists(directoryName)) {
                        logger.debug(std::format("Directory does not exist, skipping: {}", directoryName));
                        skippedDirectories++;
                        return;
                    }

                    try {
                        logger.debug(std::format("Starting recursive iteration of: {}", directoryName));
                        DIRECTORY_ITER_R recursiveDirectoryIter(directoryName, ITER_OPTIONS::skip_permission_denied, er);

                        if (er) {
                            logger.warn(std::format("Error accessing directory {}: {}", directoryName, er.message()));
                            skippedDirectories++;
                            return;
                        }

                        size_t dirFileCount = 0;
                        std::ranges::for_each(recursiveDirectoryIter, [&](const auto& entry) -> void {
                            auto pathSTR = entry.path().string();

                            // Check exclusions
                            for (const auto& excluded : constants::ExcludeDirectories) {
                                if (pathSTR.starts_with(excluded)) {
                                    logger.debug(std::format("Skipping excluded path: {}", pathSTR));
                                    return;
                                }
                            }

                            if (is_file_accessible(entry.path())) {
                                logger.debug(std::format("Adding accessible file: {}", pathSTR));
                                data.push_back(std::move(entry.path()));
                                filesAdded++;
                                dirFileCount++;
                            }
                        });

                        logger.debug(std::format("Completed directory {}: {} files added", directoryName, dirFileCount));
                    } catch (std::filesystem::filesystem_error& e) {
                        logger.warn(std::format("Filesystem error processing directory {}: {}", directoryName, e.what()));
                        skippedDirectories++;
                    } catch (const std::exception& e) {
                        logger.warn(std::format("Standard exception processing directory {}: {}", directoryName, e.what()));
                        skippedDirectories++;
                    }
                });
            });

            logger.success(std::format("System scan complete: {} files loaded from {} directories ({} skipped)",
                filesAdded, processedDirectories - skippedDirectories, skippedDirectories));

            return data;
        }
    } // namespace filesystem_utils

    namespace json_utils {
        auto read_data(const std::filesystem::path& file) -> nlohmann::json {
            auto& logger = logger::Logger::getInstance();
            logger.debug(std::format("Reading JSON data from: {}", file.string()));

            if (!exists(file)) {
                logger.error(std::format("JSON file not found: {}", file.string()));
                throw PathNotFound(file.string());
            }

            std::ifstream stream(file);
            if (!stream) {
                logger.error(std::format("Failed to open stream for JSON file: {}", file.string()));
                throw StreamOpeningError();
            }

            try {
                logger.debug("Parsing JSON data...");
                nlohmann::json json;
                stream >> json;

                logger.debug(std::format("Successfully read JSON data from: {}", file.string()));
                return json;
            } catch (const nlohmann::json::exception& e) {
                logger.error(std::format("JSON parsing error for file {}: {}", file.string(), e.what()));
                throw;
            } catch (const std::exception& e) {
                logger.error(std::format("Standard exception reading JSON from {}: {}", file.string(), e.what()));
                throw;
            }
        }

        auto write_data(const PATH &file, const JSON &json) -> void {
            auto& logger = logger::Logger::getInstance();
            logger.debug(std::format("Writing JSON data to: {}", file.string()));

            std::ofstream stream(file.string());
            if (!stream) {
                logger.error(std::format("Failed to open stream for writing JSON to: {}", file.string()));
                throw StreamOpeningError();
            }

            try {
                logger.debug("Serializing and writing JSON data...");
                stream << json.dump(4);

                if (stream.good()) {
                    logger.success(std::format("Successfully wrote JSON data to: {}", file.string()));
                } else {
                    logger.error(std::format("Stream error occurred while writing to: {}", file.string()));
                    throw StreamOpeningError();
                }
            } catch (const nlohmann::json::exception& e) {
                logger.error(std::format("JSON serialization error for file {}: {}", file.string(), e.what()));
                throw;
            } catch (const std::exception& e) {
                logger.error(std::format("Standard exception writing JSON to {}: {}", file.string(), e.what()));
                throw;
            }
        }

        auto write_data(PATH &&file, const JSON &json) -> void {
            auto& logger = logger::Logger::getInstance();
            logger.debug(std::format("Writing JSON data to (rvalue path): {}", file.string()));

            std::ofstream stream(file.string());
            if (!stream) {
                logger.error(std::format("Failed to open stream for writing JSON to: {}", file.string()));
                throw StreamOpeningError();
            }

            try {
                logger.debug("Serializing and writing JSON data...");
                stream << json.dump(4);

                if (stream.good()) {
                    logger.success(std::format("Successfully wrote JSON data to: {}", file.string()));
                } else {
                    logger.error(std::format("Stream error occurred while writing to: {}", file.string()));
                    throw StreamOpeningError();
                }
            } catch (const nlohmann::json::exception& e) {
                logger.error(std::format("JSON serialization error for file {}: {}", file.string(), e.what()));
                throw;
            } catch (const std::exception& e) {
                logger.error(std::format("Standard exception writing JSON to {}: {}", file.string(), e.what()));
                throw;
            }
        }
    } // namespace json_utils
} // namespace support