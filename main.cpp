#include "ConfigManager/include/ConfigManager.hpp"
#include "FileManager/include/IndexManager.hpp"
#include "HELPERS/include/support.hpp"
#include "Scanner/include/Scanner.hpp"
#include <CLI/CLI.hpp>
#include <algorithm>
#include <exception>
#include <filesystem>

namespace {
    void save_results_file(nlohmann::json &data, const std::string &path_name, const SCAN_RESULTS &results) {
        std::string is_detected = "detected";
        if (results.empty()) {
            is_detected = "detected";
        }
        try {
            data[path_name]["detection status"] = std::move(is_detected);
            data[path_name]["number of detected viruses"] = results.size();
            data[path_name]["viruses"] = results;
        } catch (std::exception &ERROR) { throw ERROR;
        }
    }
} // namespace
//
namespace {
    constexpr const char *INDEX_FILE = "../antivirus/AppData/index_file.json";
    const int NUMBER_OF_THREADS = support::json_utils::read_data("../antivirus/AppData/user_settings.json")["Number of threads"];
} // namespace

int main(int argc, char *argv[]) {
    CLI::App app{"Antivirus"};

    std::string file_path;
    std::string directory_path;
    std::string config_file;
    bool quick_scan_flag = false;
    bool system_scan_flag = false;
    bool update_flag = false;
    bool clear_flag = false;
    int number_of_threads = 4;

    app.add_option("--scan_file", file_path, "File scanning");
    app.add_option("--scan_directory", directory_path, "Directory scanning");
    app.add_option("--scan_config", config_file, "Custom options for scanning located in config file");
    app.add_option("--set_threads", number_of_threads, "Set number of threads for antivirus");
    app.add_flag("--quick", quick_scan_flag, "Enables quick scans");
    app.add_flag("--system", system_scan_flag, "Scans all files on your system");
    app.add_flag("--update", update_flag, "Updating metaindex with fresh data");
    app.add_flag("--clear", clear_flag, "Clearing your computer from infected files detected during last scan");

    try {
        app.parse(argc, argv);

        if (number_of_threads != 4) {
            nlohmann::json threadInfo;
            threadInfo["Number of threads"] = number_of_threads;
            support::json_utils::write_data("../antivirus/AppData/user_settings.json", threadInfo);
        }

        if (update_flag) {
            index_manager::update_metaindex(INDEX_FILE);
        }

        index_manager::PATHS_CONTAINER filesToBeScanned;

        if (!file_path.empty()) {
            filesToBeScanned.emplace_back(file_path);
        }

        if (!directory_path.empty()) {
            try {
                auto directoryPath = std::filesystem::path(directory_path);
                auto directoryFiles = support::filesystem_utils::load_from_directory(directoryPath);

                std::ranges::for_each(directoryFiles, [&](const std::filesystem::path& filePath) -> void {
                    filesToBeScanned.emplace_back(filePath);
                });

            } catch (std::exception &_) {
                throw;
            }
        }

        if (!config_file.empty()) {
                try {
                    std::filesystem::path configFile = std::filesystem::path(config_file);
                    filesToBeScanned = config_manager::fetch_config_files(configFile);

                } catch (std::exception &_) {
                    throw;
                }
            }

        if (system_scan_flag) {
            filesToBeScanned = support::filesystem_utils::load_files_from_system();
        }

        if (quick_scan_flag) {
            auto filesIndexes = support::json_utils::read_data(INDEX_FILE);
            index_manager::filterModified(filesIndexes, filesToBeScanned);
        }

        nlohmann::json OUTPUT;

        auto results = scanner::scanMultipleFiles(filesToBeScanned, NUMBER_OF_THREADS);

        std::ranges::for_each(results, [&](const std::pair<std::string, SCAN_RESULTS>& scanInfo) -> void {
            save_results_file(OUTPUT, scanInfo.first, scanInfo.second);
        });

        support::json_utils::write_data("output.json", OUTPUT);

    } catch (const CLI::ParseError &error) {
        return app.exit(error);
    }

    return 0;
}
