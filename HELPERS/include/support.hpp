#pragma once

#include "../../FileManager/include/File.hpp"
#include "../include/json.hpp"
#include <filesystem>
#include <vector>

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

        auto load_from_directory(const FILE::path &__path) -> PATHS_CONTAINER;
        auto load_files_from_system(const bool __mod = false) -> FILES_PACK;

    } // namespace filesystem_utils
    namespace json_utils {
        namespace FILE = std::filesystem;

        auto read_data(FILE::path &&__file) -> nlohmann::json;
        auto write_data(const FILE::path &__file, const nlohmann::json &__json) -> void;
        auto write_data(FILE::path &&__file, const nlohmann::json &__json) -> void;
    } // namespace json_utils
} // namespace support
