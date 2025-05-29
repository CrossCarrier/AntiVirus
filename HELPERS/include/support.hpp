#pragma once

#include "../../FileManager/include/File.hpp"
#include "../include/json.hpp"
#include <filesystem>
#include <unordered_map>
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
        auto split(const std::vector<Type> &__val, const int __chunks) -> std::vector<std::vector<Type>> {
            std::vector<std::vector<Type>> result(__chunks);

            auto chunk_size = static_cast<int>(__val.size()) / __chunks;
            auto reminder = static_cast<int>(__val.size()) % __chunks;

            int idx = 0;
            for (int i = 0; i < __chunks; i++) {
                int __size = (i == (__chunks - 1)) ? reminder : chunk_size;
                std::vector<Type> __con(__size);

                for (int ii = 0; ii < __size; ii++) {
                    __con.at(ii) = __val.at(idx++);
                }

                result.at(i) = std::move(__con);
            }

            return result;
        }
        template <typename Type1, typename Type2>
        auto operator+=(std::unordered_map<Type1, Type2> &lhs, const std::unordered_map<Type1, Type2> &rhs) -> void {
            std::ranges::for_each(rhs, [&lhs](const std::pair<Type1, Type2> &data) -> void { lhs[data.first] = data.second; });
        }

        template <typename Type1, typename Type2, MapDescriptor... Maps>
        auto mergeMaps(const std::vector<std::unordered_map<Type1, Type2>> maps) -> std::unordered_map<Type1, Type2> {
            std::unordered_map<Type1, Type2> mergedResult;
            std::ranges::for_each(maps, [&mergedResult](const auto &map) -> void { mergedResult += map; });

            return mergedResult;
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
        auto load_files_from_system() -> PATHS_CONTAINER;

    } // namespace filesystem_utils
    namespace json_utils {
        namespace FILE = std::filesystem;

        auto read_data(const FILE::path &__file) -> nlohmann::json;
        auto write_data(const FILE::path &__file, const nlohmann::json &__json) -> void;
        auto write_data(FILE::path &&__file, const nlohmann::json &__json) -> void;
    } // namespace json_utils
} // namespace support
