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

namespace {
    const auto INDEX_STORAGE = config_manager::get_index_storage_path();
    const auto OUTPUT_FILE = config_manager::get_output_file_path();
    const auto NUMBER_OF_THREADS = config_manager::get_number_of_threads();
} // namespace
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

    auto displayDetectedViruses(const std::string& OUTPUT_FILE) -> void {
        JSON outputJSONFile;
        try {
            outputJSONFile = support::json_utils::read_data(OUTPUT_FILE);
        } catch (const std::exception& ERROR) {
            std::cerr << "Problem with reading data" << std::endl;
            // LOGGING error logic
            throw;
        }

        for (const auto& item : outputJSONFile.items()) {
            const std::string& infectedFilePath = item.key();
            const nlohmann::json& detectionDetails = item.value();

            std::cout << "Infected file : " << infectedFilePath << std::endl;

            if (detectionDetails.contains("number of detected viruses") && detectionDetails["number of detected viruses"].is_number()) {
                std::cout << "Detected viruses : " << detectionDetails["number of detected viruses"].get<int>() << std::endl;
            } else {
                std::cout << "Detected viruses : N/A" << std::endl;
            }

            std::cout << "Viruses names : " << std::endl;
            if (detectionDetails.contains("viruses") && detectionDetails["viruses"].is_array()) {
                std::ranges::for_each(detectionDetails["viruses"], [&](const nlohmann::json& virusNameJson) -> void {
                    if (virusNameJson.is_string()) {
                        std::cout << "- " << virusNameJson.get<std::string>() << std::endl;
                    }
                });
            }
            std::cout << "------------------------------------" << std::endl;
        }
    }
} // namespace

int main(int argc, char *argv[]) {
    CLI::App app{"Antivirus"};

    std::string file_path;
    auto OPT_FILE_SCAN = app.add_option("--scan_file", file_path, "File scanning");
    OPT_FILE_SCAN->expected(1);

    std::string directory_path;
    auto OPT_DIRECTORY_SCAN = app.add_option("--scan_directory", directory_path, "Directory scanning");
    OPT_DIRECTORY_SCAN->expected(1);

    std::string config_file;
    auto OPT_SCAN_CONFIG = app.add_option("--scan_config", config_file, "Custom options for scanning located in config file");
    OPT_SCAN_CONFIG->expected(1);

    int number_of_threads;
    auto OPT_THREADS = app.add_option("--set_threads", number_of_threads, "Set number of threads for antivirus");
    OPT_THREADS->expected(1);

    std::string indexStoragePath;
    auto OPT_INDEX_STORAGE = app.add_option("--set_index_storage", indexStoragePath, "Set desired directory for your indexes");
    OPT_INDEX_STORAGE->expected(1);

    std::string outputFilePath;
    auto OPT_OUTPUT_PATH = app.add_option("--set_output_path", outputFilePath, "Set desired path for output file fromm scanning");
    OPT_OUTPUT_PATH->expected(1);

    std::pair<std::string, std::string> new_MetaIndexARGS;
    auto OPT_CREATE = app.add_option("--create_index", new_MetaIndexARGS, "Creating new metaindex for chosen directory");
    OPT_CREATE->expected(1);

    std::string update_MetaIndexARG;
    auto OPT_UPDATE = app.add_option("--update_index", update_MetaIndexARG, "Updating metaindex with fresh data");
    OPT_UPDATE->expected(1);

    bool show_flag = false;
    app.add_flag("--show", show_flag, "Show detected viruses");

    bool quick_scan_flag;
    app.add_flag("--quick", quick_scan_flag, "Enables quick scans");

    bool system_scan_flag;
    app.add_flag("--system", system_scan_flag, "Scans all files on your system");

    bool clean_flag;
    app.add_flag("--clear", clean_flag, "Clearing your computer from infected files detected during last scan");

    try {
        app.parse(argc, argv);

        if (OPT_CREATE->count() > 0) {
            std::string &targetPath = new_MetaIndexARGS.first;
            std::string &indexName = new_MetaIndexARGS.second;

            index_manager::createMetaIndex(targetPath, indexName);
            return 0;
        }
        if (OPT_UPDATE->count() > 0) {
            const std::string &targetPath = update_MetaIndexARG;

            index_manager::updateMetaIndex(targetPath);
            return 0;
        }
        if (OPT_INDEX_STORAGE->count() > 0) {
            config_manager::set_index_storage_location(std::move(indexStoragePath));
            return 0;
        }
        if (OPT_OUTPUT_PATH->count() > 0) {
            config_manager::set_output_file(std::move(outputFilePath));
            return 0;
        }
        if (OPT_THREADS->count() > 0) {
            config_manager::change_number_of_threads(number_of_threads);
            return 0;
        }
        if (show_flag) {
            displayDetectedViruses(OUTPUT_FILE);
            return 0;
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
            filesToBeScanned = std::move(index_manager::filterModified(filesToBeScanned));
        }

        if (clean_flag) {
            try {
                cleaner::removeInfected(OUTPUT_FILE);
            } catch (std::exception &_) {
                throw;
            }
        }

        processFiles(filesToBeScanned, OUTPUT_FILE, NUMBER_OF_THREADS);
    }
    catch (const BasicErrorBuild& ERROR) {
        std::cerr << ERROR.getTips() << std::endl;
        throw;
    }
    catch (const CLI::ParseError &error) {
        return app.exit(error);
    }

    return 0;
}
