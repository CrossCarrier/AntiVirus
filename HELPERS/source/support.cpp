#include "../include/support.hpp"
#include "../include/Constants.hpp"
#include "../../ERRORS_PACK/include/errors.hpp"
#include "../../HELPERS/include/Types.hpp"
#include <fstream>
#include <iostream>
#include <unordered_set>

namespace {
    auto is_file_accessible(const PATH &file_path) -> bool {
        std::error_code ec;

        if (!std::filesystem::exists(file_path, ec) || ec) {
            return false;
        }

        if (!std::filesystem::is_regular_file(file_path, ec) || ec) {
            return false;
        }

        return true;
    }
} // namespace

namespace support {
    namespace filesystem_utils {
        auto load_from_directory(const PATH &path) -> PATHS_CONTAINER {
            using DIR_ENTRY = std::filesystem::directory_entry;
            PATHS_CONTAINER fetched_files;

            DIRECTORY_ITER_R dir_iter(path, ITER_OPTIONS::skip_permission_denied);
            std::ranges::for_each(dir_iter, [&](const DIR_ENTRY &entry) -> void {
                if (entry.is_regular_file()) {
                    fetched_files.push_back(entry);
                }
            });
            return fetched_files;
        }

        auto load_files_from_system() -> PATHS_CONTAINER {
            PATHS_CONTAINER data;
            std::error_code er;

            std::vector<std::unordered_set<std::string>> DirectoriesToBeScanned
            {
                constants::UserDirectories,
                constants::ExcludeDirectories
            };

            std::ranges::for_each(DirectoriesToBeScanned, [&](const auto& directories) -> void {
                std::ranges::for_each(directories, [&](const auto& directoryName) -> void {

                    if (!std::filesystem::exists(directoryName)) {
                        return;
                    }

                    try {
                        DIRECTORY_ITER_R recursiveDirectoryIter(directoryName, ITER_OPTIONS::skip_permission_denied, er);

                        if (er) {
                            return;
                        }

                        std::ranges::for_each(recursiveDirectoryIter, [&](const auto& entry) -> void {

                            auto pathSTR = entry.path().string();
                            std::cout << pathSTR << std::endl;

                            for (const auto& excluded : constants::ExcludeDirectories) {
                                if (pathSTR.starts_with(excluded)) return;
                            }

                            if (is_file_accessible(entry.path())) data.push_back(std::move(entry.path()));

                        });
                    } catch (std::filesystem::filesystem_error& _) {
                    }
                });
            });

            return data;
        }
    } // namespace filesystem_utils

    namespace json_utils {
        auto read_data(const std::filesystem::path& file) -> nlohmann::json {
            if (!exists(file)) {
                throw PathNotFound(std::string("Cannot read data from file") + file.string());
            }
            std::ifstream stream(file);
            if (!stream) {
                throw StreamOpeningError(std::string("Failed to open file for reading data. File : ") + file.string());
            }
            nlohmann::json json;
            stream >> json;

            return json;
        }

        auto write_data(const PATH &file, const JSON &json) -> void {
            std::ofstream stream(file.string());
            if (!stream) {
                throw StreamOpeningError(std::string("Failed to open file for writing data. File : ") + file.c_str());
            }
            stream << json.dump(4);
        }

        auto write_data(PATH &&file, const JSON &json) -> void {
            std::ofstream stream(file.string());
            if (!stream) {
                throw StreamOpeningError(std::string("Failed to open file for writing data. File : ") + file.c_str());
            }
            stream << json.dump(4);
        }
    } // namespace json_utils
} // namespace support
