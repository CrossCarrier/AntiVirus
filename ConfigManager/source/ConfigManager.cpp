#include "../include/ConfigManager.hpp"
#include "../../HELPERS/include/support.hpp"
#include "CLI/Validators.hpp"

#include <algorithm>
#include <exception>
#include <filesystem>
#include <iostream>
#include <stdexcept>

namespace config_manager {

    CONFIG_FILES fetch_config_files(const std::filesystem::path &config_file) {
        CONFIG_FILES fetched_files;
        try {
            auto config_data = support::json_utils::read_data(config_file);

            std::vector<std::string> DIRECTORIES = config_data["Directories"];

            for (const auto& dir : DIRECTORIES) {
                std::cout << dir << std::endl;
            }

            std::vector<std::string> FILES = config_data["Files"];

            if (!DIRECTORIES.empty()) {
                std::ranges::for_each(DIRECTORIES, [&fetched_files](const std::string &directory_path) -> void {
                    auto DirPathABS = std::filesystem::absolute(directory_path);
                    std::cout << DirPathABS.string() << std::endl;

                    if (!std::filesystem::exists(DirPathABS)) {
                        throw std::invalid_argument("DIR PATH DOES NOT EXIST!");
                    }
                    if (!std::filesystem::is_directory(DirPathABS)) {
                        throw std::invalid_argument("CUSTOM ERROR NEEDED THERE!");
                    }

                    std::ranges::for_each(std::filesystem::recursive_directory_iterator(DirPathABS, ITER_OPTIONS::skip_permission_denied),
                                          [&fetched_files](const auto &entry) -> void {
                                              if (!std::filesystem::is_regular_file(entry)) {
                                                  return;
                                              }
                                              fetched_files.push_back(std::move(std::filesystem::absolute(entry)));
                                          });
                });
            }
            if (!FILES.empty()) {
                std::ranges::for_each(FILES, [&fetched_files](const std::string &file_path) -> void {
                    auto FilePathABS = std::filesystem::absolute(file_path);

                    if (!std::filesystem::exists(FilePathABS)) {
                        throw std::invalid_argument("CUSTOM ERRROR NEEDED THERE!");
                    }
                    if (!std::filesystem::is_regular_file(FilePathABS)) {
                        return;
                    }

                    fetched_files.emplace_back(FilePathABS);
                });
            }

        } catch (std::exception &ERROR) {
            std::cerr << ERROR.what() << std::endl;
            throw;
        }

        return fetched_files;
    }
} // namespace config_manager
