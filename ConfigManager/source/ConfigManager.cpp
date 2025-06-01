#include "../include/ConfigManager.hpp"
#include "../../HELPERS/include/support.hpp"
#include <algorithm>
#include <exception>
#include <filesystem>
#include <stdexcept>

namespace config_manager {

    CONFIG_FILES fetch_config_files(const std::filesystem::path &config_file) {
        CONFIG_FILES fetched_files;
        try {
            auto config_data = support::json_utils::read_data(config_file);

            std::vector<std::string> DIRECTORIES = config_data["Directories"];
            std::vector<std::string> FILES = config_data["Files"];

            if (!DIRECTORIES.empty()) {
                std::ranges::for_each(DIRECTORIES, [&fetched_files](const std::string &directory_path) -> void {
                    if (!std::filesystem::exists(directory_path)) {
                        throw std::invalid_argument("CUSTOM ERROR NEEDED THERE!");
                    }
                    if (!std::filesystem::is_directory(directory_path)) {
                        throw std::invalid_argument("CUSTOM ERROR NEEDED THERE!");
                    }

                    std::ranges::for_each(std::filesystem::recursive_directory_iterator(directory_path),
                                          [&fetched_files](const auto &entry) -> void {
                                              if (!std::filesystem::is_regular_file(entry)) {
                                                  return;
                                              }
                                              fetched_files.push_back(std::move(entry));
                                          });
                });
            }
            if (!FILES.empty()) {
                std::ranges::for_each(FILES, [&fetched_files](const std::string &file_path) -> void {
                    if (!std::filesystem::exists(file_path)) {
                        throw std::invalid_argument("CUSTOM ERRROR NEEDED THERE!");
                    }
                    if (!std::filesystem::is_regular_file(file_path)) {
                        return;
                    }

                    fetched_files.emplace_back(file_path);
                });
            }

        } catch (std::exception &ERROR) {
            throw ERROR;
        }

        return fetched_files;
    }
} // namespace config_manager
