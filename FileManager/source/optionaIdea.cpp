#include "../../HELPERS/include/json.hpp"
#include "../include/HASH-SHA256.hpp"
#include <algorithm>
#include <chrono>
#include <exception>
#include <filesystem>
#include <memory>
#include <stdexcept>

namespace {
    bool validate_file(const std::filesystem::path &_path) {
        if (!std::filesystem::exists(_path)) {
            return false;
        }
        if (!std::filesystem::is_regular_file(_path)) {
            return false;
        }
        return true;
    }
    bool validate_directory(const std::filesystem::path &_dir_path) {
        if (!std::filesystem::exists(_dir_path)) {
            return false;
        }
        if (!std::filesystem::is_directory(_dir_path)) {
            return false;
        }
        return true;
    }
} // namespace

namespace filemanager {
    namespace file {
        time_t lastModificationTime(const std::filesystem::path &_path) {
            if (!validate_file(_path))
                throw std::invalid_argument("Cannot load data from your file!");

            auto file_modification_time = std::filesystem::last_write_time(_path);
            const auto system_time = std::chrono::file_clock::to_sys(file_modification_time);
            auto converted_system_time = std::chrono::time_point_cast<std::chrono::system_clock::duration>(system_time);

            return std::chrono::system_clock::to_time_t(converted_system_time);
        }
        ssize_t size(const std::filesystem::path &_path) {
            if (!validate_file(_path))
                throw std::invalid_argument("Cannot load data from your file!");

            return std::filesystem::file_size(_path);
        }
        std::string hash(const std::filesystem::path &_path) {
            if (!validate_file(_path))
                throw std::invalid_argument("Cannot load data from you file!");

            std::string file_hash;
            try {
                file_hash = hash_SHA256::hash_file(_path);
            } catch (std::exception &ERROR) {
                throw ERROR;
            }

            return file_hash;
        }

        bool isMod(const std::filesystem::path &_path, const time_t _prev_mod, const ssize_t _prev_size,
                   const std::string &_prev_hash) {
            return (_prev_mod != lastModificationTime(_path) && _prev_size != size(_path) && _prev_hash != hash(_path));
        }
    } // namespace file
    namespace directory {
        using PATHS_CONTAINER = std::vector<std::filesystem::path>;
        using DIRECTORY_ITER_R = std::filesystem::recursive_directory_iterator;
        using ITER_OPTIONS = std::filesystem::directory_options;

        PATHS_CONTAINER loadFiles(const std::filesystem::path &__path) {
            if (!validate_directory(__path))
                throw std::invalid_argument("Invalid Directory!");

            PATHS_CONTAINER fetched_files;
            DIRECTORY_ITER_R dir_iter(__path, ITER_OPTIONS::skip_permission_denied);

            std::ranges::for_each(dir_iter, [&](const auto &__entry) -> void {
                if (validate_file(__entry)) {
                    fetched_files.push_back(__entry);
                }
            });
            return fetched_files;
        }
        PATHS_CONTAINER loadModifiedFiles(const std::filesystem::path &_path, nlohmann::json _indexData) {
            if (!validate_directory(_path))
                throw std::invalid_argument("Invalid Directory");
            PATHS_CONTAINER fetched_files;
            try {
                fetched_files = loadFiles(_path);
            } catch (std::exception &ERROR) {
                throw ERROR;
            }

            try {
                std::ranges::for_each(fetched_files, [&](const std::filesystem::path &_file) -> void {
                    std::unique_ptr<time_t> PreModeData = std::make_unique<time_t>(_indexData["Modification Time"]);
                    std::unique_ptr<ssize_t> PreSize = std::make_unique<ssize_t>(_indexData["Size"]);
                    std::unique_ptr<std::string> PreHash = std::make_unique<std::string>(_indexData["Hash"]);

                    if (!file::isMod(_file, *PreModeData, *PreSize, *PreHash)) {
                        fetched_files.erase(std::ranges::find(fetched_files, _file));
                    } else {
                        return;
                    }
                });
            } catch (std::exception &ERROR) {
                throw ERROR;
            }

            return fetched_files;
        }
    } // namespace directory
} // namespace filemanager
