#include "../include/IndexManager.hpp"
#include "../../HELPERS/include/support.hpp"
#include "../include/FileManager.hpp"
#include <algorithm>
#include <unordered_set>
#include <iostream>
#include "../../HELPERS/include/Types.hpp"
#include "CLI/Validators.hpp"

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

    auto isQuickLocation(const PATH& filePath) -> bool {
        return std::ranges::any_of(filePath, [&](const auto& part) -> bool {
            return (quick_locations.contains(part.string()));
        });
    }

} // namespace

namespace {
    template <typename T>
    auto add_property(JSON &json, const std::string &key, T value) -> void {
        json[key] = value;
    }

    auto create_file_entry(const PATH &file) -> JSON {
        JSON entry;

        add_property(entry, "Hash", filemanager::file::hash(file));
        add_property(entry, "Modification time", filemanager::file::lastModificationTime(file));
        add_property(entry, "Size", filemanager::file::size(file));

        return entry;
    }

    auto update_json_with_files(JSON &json, const PATHS_CONTAINER &files) -> void {
        std::ranges::for_each(files, [&](const PATH & val) -> void {
            json[val.c_str()] = create_file_entry(val);
        });
    }
} // namespace
namespace index_manager {

    auto update_metaindex(PATH &&path /* Path to file, where files are indexed*/) -> void {

        JSON data;

        update_json_with_files(data, support::filesystem_utils::load_files_from_system());

        support::json_utils::write_data(path, data);
    }

    auto filterModified(const JSON &data, PATHS_CONTAINER &files) -> void {
        std::erase_if(files, [&](const PATH& filePath) -> bool {

            // if (!quick_extensions.contains(filePath.extension().c_str())) { return true; }
            // if (!isQuickLocation(filePath)) { return true; }

            auto absPATH = std::filesystem::absolute(filePath);
            std::cout << "CHECKING FOR MOD : " << absPATH.string() << std::endl;

            try {

                time_t lastModTime = data[absPATH.string()]["Modification time"];
                ssize_t lastSize = data[absPATH.string()]["Size"];
                std::string lastHash = data[absPATH.string()]["Hash"];

                if (!filemanager::file::isMod(filePath, lastModTime, lastSize, lastHash)) {
                    return true;
                }
            } catch (std::exception& _) {
                throw;
            }

            return false;
        });
    }
} // namespace index_manager
