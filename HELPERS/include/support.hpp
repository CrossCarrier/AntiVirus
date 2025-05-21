#pragma once

#include "../include/json.hpp"
#include <algorithm>
#include <filesystem>
#include <vector>

#include "../../ERRORS_PACK/include/errors.hpp"
#include "../../FileManager/include/File.hpp"
#include <filesystem>
#include <fstream>

namespace {
    template <typename Type>
    concept MapDescriptor =
        requires {
            typename Type::key_type;
            typename Type::mapped_type;
            typename Type::iterator;
        } && (std::is_same_v<Type, std::map<typename Type::key_type, typename Type::mapped_type>> ||
              std::is_same_v<Type, std::unordered_map<typename Type::key_type, typename Type::mapped_type>>);
} // namespace

namespace support {
    namespace container_utils {
        template <typename Type>
        auto split(std::vector<Type> &&__val, int &&__chunks) -> std::vector<std::vector<Type>> {
            std::vector<Type> result(__chunks);

            auto chunk_size = static_cast<int>(__val.size()) / __chunks;
            auto reminder = static_cast<int>(__val.size()) % __chunks;

            int idx = 0;
            for (size_t i = 0; i < __chunks; i++) {
                int __size = (i == (__chunks - 1)) ? reminder : chunk_size;
                std::vector<Type> __con(__size);
                for (size_t ii = 0; ii < __size; ii++) {
                    __con.at(ii) = __val.at(idx++);
                }
            }

            return result;
        }
    } // namespace container_utils
    namespace filesystem_utils {
        namespace FILE = std::filesystem;

        using PATHS_CONTAINER = std::vector<std::filesystem::path>;
        using DIRECTORY_ITER_R = FILE::recursive_directory_iterator;
        using DIRECTORY_ITER = FILE::directory_iterator;
        using ITER_OPTIONS = FILE::directory_options;
        using FILES_PACK = std::vector<File>;

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
        auto load_files_from_system(const bool __mod = false) -> FILES_PACK {
            FILES_PACK __data;
            std::error_code er;
            DIRECTORY_ITER_R iter("/", ITER_OPTIONS::skip_permission_denied, er);

            std::ranges::for_each(iter, [&](const auto &entry) -> void {
                if (er) {
                    return;
                }
                if (FILE::is_regular_file(entry, er)) {
                    __data.emplace_back(entry.path());
                }
            });

            return __data;
        }

    } // namespace filesystem_utils
    namespace json_utils {
        namespace FILE = std::filesystem;

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

        template <typename IterInp>
        auto write_data(FILE::path &&__file, IterInp __first, const IterInp &__last) -> void {
            std::ofstream stream(__file.string());
            if (!stream) {
                throw StreamOpeningError(std::string("Failed to open file for writing data. File : ") + __file.c_str());
            }
            nlohmann::json json;
            json.parse(__first, __last);
        }

        auto write_data(const FILE::path &__file, const nlohmann::json &__json) -> void {
            std::ofstream stream(__file.string());
            if (!stream) {
                throw StreamOpeningError(std::string("Failed to open file for writing data. File : ") + __file.c_str());
            }
            stream << __json;
        }
        auto write_data(FILE::path &&__file, const nlohmann::json &__json) -> void {
            std::ofstream stream(__file.string());
            if (!stream) {
                throw StreamOpeningError(std::string("Failed to open file for writing data. File : ") + __file.c_str());
            }
            stream << __json;
        }
    } // namespace json_utils
} // namespace support