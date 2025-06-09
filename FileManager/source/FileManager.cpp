#include "../include/FileManager.hpp"
#include "../include/HASH-SHA256.hpp"
#include "../../ERRORS_PACK/include/errors.hpp"
#include "../../Logger/include/Logger.hpp"
#include <chrono>
#include <exception>
#include <iostream>
#include <format>

namespace filemanager {

    namespace validate {

        auto validate_file(const std::filesystem::path &_path) -> bool {
            auto& logger = logger::Logger::getInstance();
            logger.debug(std::format("Validating file: {}", _path.string()));

            if (!std::filesystem::exists(_path)) {
                logger.warn(std::format("File does not exist: {}", _path.string()));
                return false;
            }

            if (!std::filesystem::is_regular_file(_path)) {
                logger.warn(std::format("Path is not a regular file: {}", _path.string()));
                return false;
            }

            logger.debug(std::format("File validation successful: {}", _path.string()));
            return true;
        }

        auto validate_directory(const std::filesystem::path &_dir_path) -> bool {
            auto& logger = logger::Logger::getInstance();
            logger.debug(std::format("Validating directory: {}", _dir_path.string()));

            if (!std::filesystem::exists(_dir_path)) {
                logger.warn(std::format("Directory does not exist: {}", _dir_path.string()));
                return false;
            }

            if (!std::filesystem::is_directory(_dir_path)) {
                logger.warn(std::format("Path is not a directory: {}", _dir_path.string()));
                return false;
            }

            logger.debug(std::format("Directory validation successful: {}", _dir_path.string()));
            return true;
        }
    }

    namespace file {

        auto lastModificationTime(const std::filesystem::path &_path) -> time_t {
            auto& logger = logger::Logger::getInstance();
            logger.debug(std::format("Getting last modification time for: {}", _path.string()));

            if (!validate::validate_file(_path)) {
                logger.error(std::format("File validation failed: {}", _path.string()));
                throw FileValidationError(_path.string());
            }

            try {
                logger.debug("Retrieving file modification time...");
                const auto file_modification_time = std::filesystem::last_write_time(_path);
                const auto system_time = std::chrono::file_clock::to_sys(file_modification_time);
                const auto converted_system_time = std::chrono::time_point_cast<std::chrono::system_clock::duration>(system_time);
                const auto time_t_result = std::chrono::system_clock::to_time_t(converted_system_time);

                logger.debug(std::format("Last modification time retrieved successfully: {}", time_t_result));
                return time_t_result;
            } catch (const std::filesystem::filesystem_error& e) {
                logger.error(std::format("Filesystem error getting modification time: {}", e.what()));
                throw;
            } catch (const std::exception& e) {
                logger.error(std::format("Error getting modification time: {}", e.what()));
                throw;
            }
        }

        auto size(const std::filesystem::path &_path) -> uintmax_t {
            auto& logger = logger::Logger::getInstance();
            logger.debug(std::format("Getting size for file: {}", _path.string()));

            if (!validate::validate_file(_path)) {
                logger.error(std::format("File validation failed: {}", _path.string()));
                throw FileValidationError(_path.string());
            }

            try {
                logger.debug("Retrieving file size...");
                const auto size_result = std::filesystem::file_size(_path);
                logger.debug(std::format("File size retrieved successfully: {} bytes", size_result));
                return size_result;
            } catch (const std::filesystem::filesystem_error& e) {
                logger.error(std::format("Filesystem error getting file size: {}", e.what()));
                throw;
            } catch (const std::exception& e) {
                logger.error(std::format("Error getting file size: {}", e.what()));
                throw;
            }
        }

        auto hash(const std::filesystem::path &_path) -> std::string {
            auto& logger = logger::Logger::getInstance();
            logger.debug(std::format("Calculating hash for file: {}", _path.string()));

            if (!validate::validate_file(_path)) {
                logger.error(std::format("File validation failed: {}", _path.string()));
                throw FileValidationError(_path.string());
            }

            std::string file_hash;
            try {
                logger.debug("Computing SHA-256 hash...");
                file_hash = hash_SHA256::hash_file(_path);
                logger.debug(std::format("Hash calculation successful: {}", file_hash));
                return file_hash;
            } catch (const std::exception& e) {
                logger.error(std::format("Error calculating file hash: {}", e.what()));
                throw;
            }
        }

        auto isMod(const std::filesystem::path &_path, const time_t _prev_mod, const ssize_t _prev_size,
                   const std::string &_prev_hash) -> bool {
            auto& logger = logger::Logger::getInstance();
            logger.debug(std::format("Checking if file has been modified: {}", _path.string()));

            try {
                logger.debug(std::format("Previous values - Mod time: {}, Size: {}, Hash: {}",
                    _prev_mod, _prev_size, _prev_hash));

                logger.debug("Getting current modification time...");
                const auto current_mod_time = lastModificationTime(_path);

                logger.debug("Getting current file size...");
                const auto current_size = size(_path);

                logger.debug("Getting current file hash...");
                const auto current_hash = hash(_path);

                logger.debug(std::format("Current values - Mod time: {}, Size: {}, Hash: {}",
                    current_mod_time, current_size, current_hash));

                const bool is_modified = (_prev_mod != current_mod_time &&
                                          _prev_size != current_size &&
                                          _prev_hash != current_hash);

                if (is_modified) {
                    logger.info(std::format("File has been modified: {}", _path.string()));
                } else {
                    logger.debug(std::format("File has not been modified: {}", _path.string()));
                }

                return is_modified;
            } catch (const std::exception& e) {
                logger.error(std::format("Error checking file modification: {}", e.what()));
                throw;
            }
        }
    } // namespace file
} // namespace filemanager