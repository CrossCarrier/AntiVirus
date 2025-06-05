#include "Cleaner/include/Cleaner.hpp"
#include "ConfigManager/include/ConfigManager.hpp"
#include "ERRORS_PACK/include/errors.hpp"
#include "FileManager/include/IndexManager.hpp"
#include "HELPERS/include/support.hpp"
#include "Scanner/include/Scanner.hpp"
#include <CLI/CLI.hpp>
#include <algorithm>
#include <exception>
#include <filesystem>
#include <utility>

#include "HELPERS/include/Constants.hpp"

namespace {

    auto save_results_file(nlohmann::json &data, const std::string &path_name, SCAN_RESULTS &results) -> void {
        std::string is_detected = "detected";
        if (results.empty()) {
            is_detected = "undetected";
        }
        try {
            data[path_name]["detection status"] = std::move(is_detected);
            data[path_name]["number of detected viruses"] = results.size();
            data[path_name]["viruses"] = std::move(results);
        } catch (std::exception &_) {
            throw;
        }
    }

    auto processFiles(const PATHS_CONTAINER &files, const std::string &outputFile, int threadsNum) -> void {
        JSON scanResultsJson;

        auto results = scanner::scanMultipleFiles(files, threadsNum);

        std::ranges::for_each(results, [&](const std::pair<std::string, SCAN_RESULTS> &scanInfo) -> void {
            save_results_file(scanResultsJson, scanInfo.first, const_cast<SCAN_RESULTS &>(scanInfo.second));
        });

        support::json_utils::write_data(outputFile, scanResultsJson);
    }

} // namespace
//
namespace {
    constexpr const char *OUTPUT_FILE = "output.json";
    constexpr const char *INDEX_FILE = "../antivirus/AppData/index_file.json";
} // namespace

int main(int argc, char *argv[]) {
    CLI::App app{"Antivirus"};

    std::string file_path;
    std::string directory_path;
    std::string config_file;

    bool quick_scan_flag = false;
    bool system_scan_flag = false;
    bool update_flag = false;
    bool clean_flag = false;
    bool show_flag = false;
    int number_of_threads = 0;

    app.add_option("--scan_file", file_path, "File scanning");
    app.add_option("--scan_directory", directory_path, "Directory scanning");
    app.add_option("--scan_config", config_file, "Custom options for scanning located in config file");
    app.add_option("--set_threads", number_of_threads, "Set number of threads for antivirus");

    std::pair<std::string, std::string> new_MetaIndexARGS;
    auto OPT_CREATE = app.add_option("--create_index", new_MetaIndexARGS, "Creating new metaindex for choosed directory");
    OPT_CREATE->expected(1);

    std::string update_MetaIndexARG;
    auto OPT_UPDATE = app.add_option("--update_index", update_MetaIndexARG, "Updating metaindex with fresh data");
    OPT_UPDATE->expected(1);

    app.add_flag("--show", show_flag, "Show detected viruses");
    app.add_flag("--quick", quick_scan_flag, "Enables quick scans");
    app.add_flag("--system", system_scan_flag, "Scans all files on your system");
    app.add_flag("--clear", clean_flag, "Clearing your computer from infected files detected during last scan");

    int NUMBER_OF_THREADS;

    try {
        NUMBER_OF_THREADS = support::json_utils::read_data("../antivirus/AppData/user_settings.json")["Number of threads"];
    } catch (std::exception &e) {
        std::cerr << "Problem with reading data from user settings" << std::endl;
        throw ReadingSettingsError();
    }

    try {
        app.parse(argc, argv);

        if (number_of_threads > 0) {
            nlohmann::json threadInfo;
            threadInfo["Number of threads"] = number_of_threads;
            support::json_utils::write_data("../antivirus/AppData/user_settings.json", threadInfo);
        }

        if (OPT_CREATE->count() > 0) {
            std::string &targetPath = new_MetaIndexARGS.first;
            std::string &indexName = new_MetaIndexARGS.second;

            index_manager::createMetaIndex(targetPath, indexName);
        }
        if (OPT_UPDATE->count() > 0) {
            const std::string &targetPath = update_MetaIndexARG;

            index_manager::updateMetaIndex(targetPath);
        }

        PATHS_CONTAINER filesToBeScanned;

        if (!file_path.empty()) {
            filesToBeScanned.emplace_back(file_path);
        }

        if (!directory_path.empty()) {
            auto directoryPath = std::filesystem::path(directory_path);
            auto directoryFiles = support::filesystem_utils::load_from_directory(directoryPath);

            std::ranges::for_each(
                directoryFiles, [&](const std::filesystem::path &filePath) -> void { filesToBeScanned.emplace_back(filePath); });
        }

        if (!config_file.empty()) {
            filesToBeScanned = std::move(config_manager::fetch_config_files(std::filesystem::path(config_file)));
        }

        if (system_scan_flag) {
            filesToBeScanned = std::move(support::filesystem_utils::load_files_from_system());
        }

        if (quick_scan_flag) {
            index_manager::filterModified(support::json_utils::read_data(INDEX_FILE), filesToBeScanned);
        }

        if (clean_flag) {
            try {
                cleaner::removeInfected("output.json");
            } catch (std::exception &_) {
                throw;
            }
        }

        if (!filesToBeScanned.empty()) {
            processFiles(filesToBeScanned, OUTPUT_FILE, NUMBER_OF_THREADS);
        }

    } catch (const CLI::ParseError &error) {
        return app.exit(error);
    }

    return 0;
}
