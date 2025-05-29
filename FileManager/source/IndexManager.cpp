#include "../include/IndexManager.hpp"
#include "../../HELPERS/include/support.hpp"
#include "../include/FileManager.hpp"
#include <algorithm>

namespace {
    template <typename T>
    auto add_property(nlohmann::json &json, const std::string &key, T value) -> void {
        json[key] = value;
    }

    auto create_file_entry(const std::filesystem::path &file) -> nlohmann::json {
        nlohmann::json entry;
        add_property(entry, "Hash", "Example hash");
        add_property(entry, "Modification time", static_cast<time_t>(2123123));
        add_property(entry, "Size", static_cast<ssize_t>(12397612));
        return entry;
    }

    auto update_json_with_files(nlohmann::json & json, const std::vector<std::filesystem::path> &files) -> void {
        std::ranges::for_each(files, [&](const std::filesystem::path & val) -> void {
            json[val.c_str()] = create_file_entry(val);
        });
    }
} // namespace
namespace index_manager {

    auto update_metaindex(FILE::path &&path /* Path to file, where files are indexed*/) -> void {

        nlohmann::json data;

        update_json_with_files(data, support::filesystem_utils::load_files_from_system());

        support::json_utils::write_data(path, data);
    }

    auto filterModified(const nlohmann::json &data, PATHS_CONTAINER &files) -> void {
        std::ranges::for_each(files, [&](const std::filesystem::path& filePath) -> void {

            try {

                auto lastModTime = data[filePath.c_str()]["Modification Time"];
                auto lastSize = data[filePath.c_str()]["Size"];
                auto lastHash = data[filePath.c_str()]["Hash"];

                if (!filemanager::file::isMod(filePath, std::move(lastModTime), std::move(lastSize), std::move(lastSize))) {

                    auto it = std::ranges::find(files, filePath);

                    try {
                        files.erase(it);
                    } catch (std::exception& _) {
                        throw;
                    }
                }
            } catch (std::exception& _) {
                throw;
            }
        });
    }
} // namespace index_manager
