#include "../include/support.hpp"

namespace {
    auto is_file_accessible(const std::filesystem::path &file_path) -> bool {
        std::error_code ec;

        if (!std::filesystem::exists(file_path, ec) || ec) {
            return false;
        }

        if (!std::filesystem::is_regular_file(file_path, ec) || ec) {
            return false;
        }

        auto perms = std::filesystem::status(file_path, ec).permissions();
        if (ec || (perms & std::filesystem::perms::owner_read) == std::filesystem::perms::none) {
            return false;
        }

        std::ifstream test_file(file_path, std::ios::binary);
        if (!test_file.is_open()) {
            return false;
        }

        return true;
    }
} // namespace

namespace support {
    namespace filesystem_utils {
        auto load_from_directory(const FILE::path &__path) -> PATHS_CONTAINER {
            using DIR_ENTRY = FILE::directory_entry;
            PATHS_CONTAINER fetched_files;

            DIRECTORY_ITER_R dir_iter(__path, ITER_OPTIONS::skip_permission_denied);
            std::ranges::for_each(dir_iter, [&](const DIR_ENTRY &__entry) -> void {
                if (__entry.is_regular_file()) {
                    fetched_files.push_back(__entry);
                }
            });
            return fetched_files;
        }

        auto load_files_from_system(const bool __mod) -> FILES_PACK {
            FILES_PACK __data;
            std::error_code er;
            DIRECTORY_ITER_R iter("/", ITER_OPTIONS::skip_permission_denied, er);

            std::ranges::for_each(iter, [&](const auto &entry) -> void {
                if (er) {
                    return;
                }
                if (FILE::is_regular_file(entry, er) && !er) {
                    if (is_file_accessible(entry.path())) {
                        __data.emplace_back(entry.path());
                    }
                }
            });

            return __data;
        }
    } // namespace filesystem_utils

    namespace json_utils {
        auto read_data(FILE::path &&__file) -> nlohmann::json {
            if (!exists(__file)) {
                throw PathNotFound(std::string("Cannot read data from file") + __file.c_str());
            }
            std::ifstream stream(std::move(__file));
            if (!stream) {
                throw StreamOpeningError(std::string("Failed to open file for reading data. File : ") + __file.c_str());
            }
            nlohmann::json json;
            stream >> json;

            return json;
        }

        auto write_data(const FILE::path &__file, const nlohmann::json &__json) -> void {
            std::ofstream stream(__file.string());
            if (!stream) {
                throw StreamOpeningError(std::string("Failed to open file for writing data. File : ") + __file.c_str());
            }
            stream << __json.dump(4);
        }

        auto write_data(FILE::path &&__file, const nlohmann::json &__json) -> void {
            std::ofstream stream(__file.string());
            if (!stream) {
                throw StreamOpeningError(std::string("Failed to open file for writing data. File : ") + __file.c_str());
            }
            stream << __json.dump(4);
        }
    } // namespace json_utils
} // namespace support
