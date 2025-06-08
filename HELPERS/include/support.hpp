#pragma once

#include <unordered_set>

#include "Types.hpp"

using namespace types::filesystem_types;
using namespace types::json;

namespace support {
    namespace container_utils {

        template <typename Type>
        auto split(const std::vector<Type> &val, const int chunks) -> std::vector<std::vector<Type>> {
            std::vector<std::vector<Type>> result(chunks);

            auto chunk_size = static_cast<int>(val.size()) / chunks;
            auto reminder = static_cast<int>(val.size()) % chunks;

            int idx = 0;
            for (int i = 0; i < chunks; i++) {
                int size = (i == (chunks - 1)) ? reminder : chunk_size;
                std::vector<Type> con(size);

                for (int ii = 0; ii < size; ii++) {
                    con.at(ii) = val.at(idx++);
                }

                result.at(i) = std::move(con);
            }

            return result;
        }

        template <typename Type1, typename Type2>
        auto operator+=(std::unordered_map<Type1, Type2> &lhs, const std::unordered_map<Type1, Type2> &rhs) -> void {
            std::ranges::for_each(rhs, [&lhs](const std::pair<Type1, Type2> &data) -> void { lhs[data.first] = data.second; });
        }

        template <typename Type1, typename Type2>
        auto mergeMaps(const std::vector<std::unordered_map<Type1, Type2>> maps) -> std::unordered_map<Type1, Type2> {
            std::unordered_map<Type1, Type2> mergedResult;
            std::ranges::for_each(maps, [&mergedResult](const auto &map) -> void { mergedResult += map; });

            return mergedResult;
        }

    } // namespace container_utils
    namespace filesystem_utils {

        auto load_from_directory(const PATH &path) -> PATHS_CONTAINER;
        auto load_files_from_system() -> PATHS_CONTAINER;

    } // namespace filesystem_utils
    namespace json_utils {

        auto read_data(const PATH &file) -> JSON;
        auto write_data(const PATH &file, const JSON &json) -> void;
        auto write_data(PATH &&file, const JSON &json) -> void;

    } // namespace json_utils

} // namespace support
