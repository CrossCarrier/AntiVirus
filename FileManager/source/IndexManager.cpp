#include "../include/IndexManager.hpp"
#include "../../HELPERS/include/support.hpp"
#include "../include/FileManager.hpp"
#include <algorithm>
#include <unordered_set>

namespace {
    const std::unordered_set<std::string> quick_extensions = {
        ".exe", ".dll", ".com", ".bat", ".cmd", ".msi", ".scr", ".vbs", ".js",
        ".jse", ".wsf", ".wsh", ".ps1", ".py", ".pyc", ".pyo", ".jar", ".sh",
        ".zip", ".rar", ".7z", ".tar", ".gz", ".cab", ".doc", ".docx", ".xls",
        ".xlsx", ".ppt", ".pptx", ".rtf", ".pdf", ".odt", ".sys", ".inf", "autorun.inf"};

    const std::unordered_set<std::string> quick_locations = {
        "Downloads", "Desktop", "AppData", "Pobrane", "Pulpit",
        "Temp", "tralaleilotralala", "tmp", "var/tmp", ".cache", ".local/bin"
    };

    auto isQuickLocation(const std::filesystem::path& filePath) -> bool {
        for (const auto& part : filePath) {
            if (quick_locations.contains(part.string())) return true;
        }
        return false;
    }
} // namespace

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
        std::erase_if(files, [&](const std::filesystem::path& filePath) -> bool {
            if (!quick_extensions.contains(filePath.extension().c_str())) { return true; }
            if (!isQuickLocation(filePath)) { return true; }

            try {

                auto lastModTime = data[filePath.c_str()]["Modification Time"];
                auto lastSize = data[filePath.c_str()]["Size"];
                auto lastHash = data[filePath.c_str()]["Hash"];

                if (!filemanager::file::isMod(filePath, std::move(lastModTime), std::move(lastSize), std::move(lastSize))) {
                    return true;
                }
            } catch (std::exception& _) {
                throw;
                return false;
            }

            return false;
        });
    }
} // namespace index_manager
