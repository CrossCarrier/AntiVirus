#include "Cleaner/include/Cleaner.hpp"
#include "ConfigManager/include/ConfigManager.hpp"
#include "ERRORS_PACK/include/errors.hpp"
#include "FileManager/include/IndexManager.hpp"
#include "HELPERS/include/support.hpp"
#include "Scanner/include/Scanner.hpp"
#include "Logger/include/Logger.hpp"
#include "HELPERS/include/Types.hpp"
#include <CLI/CLI.hpp>
#include <algorithm>
#include <exception>
#include <filesystem>
#include <utility>
#include <chrono>

namespace {
    const auto OUTPUT_FILE = config_manager::get_output_file_path();
    const auto NUMBER_OF_THREADS = config_manager::get_number_of_threads();
} // namespace

namespace {
    using namespace types::filesystem_types;
    using namespace types::scanning_types;

    auto save_results_file(nlohmann::json &data, const std::string &path_name, SCAN_RESULTS &results) -> void {
        std::string is_detected = "detected";
        if (results.empty())
        {
            is_detected = "undetected";
        }

        if (!results.empty())
        {
            for (const auto& threat : results) {
                logger::Logger::getInstance().logThreatDetected(path_name, threat);
            }
        }

        try
        {
            data[path_name]["detection status"] = std::move(is_detected);
            data[path_name]["number of detected viruses"] = results.size();
            data[path_name]["viruses"] = std::move(results);
        }
        catch (std::exception &e)
        {
            logger::Logger::getInstance().logScanError(path_name, e.what());
            throw;
        }
    }

    auto processFiles(const PATHS_CONTAINER &files, const std::string &outputFile, const int threadsNum) -> void
    {
        auto& logger = logger::Logger::getInstance();

        logger::ScanType scanType = logger::ScanType::FILE_SCAN;
        if (files.size() > 100)
        {
            scanType = logger::ScanType::SYSTEM_SCAN;
        }
        else if (files.size() > 10)
        {
            scanType = logger::ScanType::DIRECTORY_SCAN;
        }

        const auto start_time = std::chrono::high_resolution_clock::now();

        logger.logScanStart(scanType, files.empty() ? "" : files[0].string());

        JSON scanResultsJson;
        logger::ScanResult scan_result;
        scan_result.files_scanned = static_cast<int>(files.size());

        try
        {
            auto results = scanner::scanMultipleFiles(files, threadsNum);

            int total_threats = 0;
            int current_file = 0;

            std::ranges::for_each(results, [&](const std::pair<std::string, SCAN_RESULTS> &scanInfo) -> void
            {
                current_file++;
                logger.logScanProgress(scanInfo.first, current_file, static_cast<int>(files.size()));

                auto& scan_results = const_cast<SCAN_RESULTS&>(scanInfo.second);
                total_threats += static_cast<int>(scan_results.size());

                save_results_file(scanResultsJson, scanInfo.first, scan_results);
            });

            support::json_utils::write_data(outputFile, scanResultsJson);

            const auto end_time = std::chrono::high_resolution_clock::now();
            scan_result.scan_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
            scan_result.threats_found = total_threats;
            scan_result.success = true;

            logger.logScanResult(scanType, scan_result);

        }
        catch (const std::exception& e)
        {
            scan_result.success = false;
            scan_result.error_message = e.what();
            logger.logScanResult(scanType, scan_result);
            throw;
        }
    }

    auto displayDetectedViruses(const std::string& OUTPUT_FILE) -> void {
        auto& logger = logger::Logger::getInstance();

        JSON outputJSONFile;
        try
        {
            outputJSONFile = support::json_utils::read_data(OUTPUT_FILE);
            logger.info(std::format("Displaying detection results from: {} ", OUTPUT_FILE));
        }
        catch (const std::exception& _)
        {
            logger.error(std::format("Problem with reading data from: {} ", OUTPUT_FILE));
            throw;
        }

        bool has_infections = false;
        for (const auto& item : outputJSONFile.items())
        {
            const std::string& infectedFilePath = item.key();
            const nlohmann::json& detectionDetails = item.value();

            if (detectionDetails.contains("detection status") && detectionDetails["detection status"].get<std::string>() == "detected")
            {
                has_infections = true;
                logger.warn(std::format("Infected file: {}", infectedFilePath));

                if (detectionDetails.contains("number of detected viruses") && detectionDetails["number of detected viruses"].is_number())
                {
                    logger.warn(std::format("Detected viruses: {}", detectionDetails["number of detected viruses"].get<int>()));
                }
                else
                {
                    logger.info("Detected viruses: N/A");
                }

                logger.warn("Viruses names:");
                if (detectionDetails.contains("viruses") && detectionDetails["viruses"].is_array())
                {
                    std::ranges::for_each(detectionDetails["viruses"], [&](const nlohmann::json& virusNameJson) -> void
                    {
                        if (virusNameJson.is_string())
                        {
                            logger.warn(std::format("- {}", virusNameJson.get<std::string>()));
                        }
                    });
                }
                logger.info("------------------------------------");
            }
        }

        if (!has_infections)
        {
            logger.success("No infected files found in scan results");
        }
    }

    auto performCleaning(const std::string& outputFile) -> void
    {
        auto& logger = logger::Logger::getInstance();
        logger.logCleaningStart();

        logger::CleaningResult clean_result;

        try
        {
            JSON infected = support::json_utils::read_data(outputFile);

            for (auto& [filePath, detectionInfo] : infected.items())
            {
                if (!detectionInfo.contains("detection status") || detectionInfo["detection status"].get<std::string>() != "detected")
                {
                    continue;
                }

                logger.logFileCleaningAttempt(filePath);

                try
                {
                    index_manager::updateAfterRemoval(filePath);
                    std::filesystem::remove(filePath);

                    logger.logFileCleaningSuccess(filePath);
                    clean_result.files_cleaned++;

                }
                catch (const std::exception& e)
                {
                    logger.logFileCleaningFailed(filePath, e.what());
                    clean_result.files_failed++;
                }
            }

            clean_result.success = true;
            logger.logCleaningResult(clean_result);

        }
        catch (const std::exception& e)
        {
            clean_result.success = false;
            clean_result.error_message = e.what();
            logger.logCleaningResult(clean_result);
            throw;
        }
    }
} // namespace

int main(int argc, char *argv[]) {
    auto& logger = logger::Logger::getInstance();

    logger.enableDebugMode(false);

    logger.logAntivirusStart();

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

    bool quick_scan_flag = false;
    app.add_flag("--quick", quick_scan_flag, "Enables quick scans");

    bool system_scan_flag = false;
    app.add_flag("--system", system_scan_flag, "Scans all files on your system");

    bool clean_flag = false;
    app.add_flag("--clear", clean_flag, "Clearing your computer from infected files detected during last scan");

    bool debug_flag = false;
    app.add_flag("--debug", debug_flag, "Enable debug logging");

    try
    {
        app.parse(argc, argv);

        if (debug_flag)
        {
            logger.enableDebugMode(true);
        }

        if (OPT_CREATE->count() > 0)
        {
            std::string &targetPath = new_MetaIndexARGS.first;
            std::string &indexName = new_MetaIndexARGS.second;

            logger.logIndexCreation(indexName, targetPath);
            try
            {
                index_manager::createMetaIndex(targetPath, indexName);
                logger.success("Index " + indexName + " created successfully");
            }
            catch (const std::exception& e)
            {
                logger.logIndexError("CREATE", e.what());
                throw;
            }
            return 0;
        }

        if (OPT_UPDATE->count() > 0)
        {
            const std::string &targetPath = update_MetaIndexARG;

            logger::IndexUpdateResult update_result;
            update_result.index_path = targetPath;
            update_result.update_time = std::chrono::system_clock::now();

            try
            {
                index_manager::updateMetaIndex(targetPath);
                update_result.success = true;
                update_result.save_location = "Index storage location";
                logger.logIndexUpdate(update_result);
            }
            catch (const std::exception& e)
            {
                update_result.success = false;
                update_result.error_message = e.what();
                logger.logIndexUpdate(update_result);
                throw;
            }
            return 0;
        }

        if (OPT_INDEX_STORAGE->count() > 0)
        {
            logger.info("Setting index storage location to: " + indexStoragePath);
            try
            {
                config_manager::set_index_storage_location(std::move(indexStoragePath));
                logger.success("Index storage location updated successfully");
            }
            catch (const std::exception& e)
            {
                logger.error("Failed to set index storage location: " + std::string(e.what()));
                throw;
            }
            return 0;
        }

        if (OPT_OUTPUT_PATH->count() > 0)
        {
            logger.info("Setting output file path to: " + outputFilePath);
            try
            {
                config_manager::set_output_file(std::move(outputFilePath));
                logger.success("Output file path updated successfully");
            }
            catch (const std::exception& e)
            {
                logger.error("Failed to set output file path: " + std::string(e.what()));
                throw;
            }
            return 0;
        }

        if (OPT_THREADS->count() > 0)
        {
            logger.info("Setting number of threads to: " + number_of_threads);
            try
            {
                config_manager::change_number_of_threads(number_of_threads);
                logger.success("Thread count updated to: " + number_of_threads);
            }
            catch (const std::exception& e)
            {
                logger.error("Failed to set thread count: " + std::string(e.what()));
                throw;
            }
            return 0;
        }

        if (show_flag)
        {
            displayDetectedViruses(OUTPUT_FILE);
            return 0;
        }

        PATHS_CONTAINER filesToBeScanned;

        if (!file_path.empty())
        {
            logger.debug("Adding single file to scan queue: " + file_path);
            filesToBeScanned.emplace_back(file_path);
        }

        if (!directory_path.empty())
        {
            logger.info("Loading files from directory: " + directory_path);
            auto directoryPath = std::filesystem::path(directory_path);
            try
            {
                auto directoryFiles = support::filesystem_utils::load_from_directory(directoryPath);
                logger.debug("Found " + std::to_string(directoryFiles.size()) + " files in directory");

                std::ranges::for_each(directoryFiles, [&](const std::filesystem::path &filePath) -> void
                    {
                        filesToBeScanned.emplace_back(filePath);
                    });
            }
            catch (const std::exception& e)
            {
                logger.error("Failed to load files from directory " + directory_path + ": " + std::string(e.what()));
                throw;
            }
        }

        if (!config_file.empty())
        {
            logger.info("Loading files from config: " + config_file);
            try
            {
                filesToBeScanned = std::move(config_manager::fetch_config_files(std::filesystem::path(config_file)));
                logger.debug("Loaded " + std::to_string(filesToBeScanned.size()) + " files from config");
            }
            catch (const std::exception& e)
            {
                logger.error("Failed to load config file " + config_file + ": " + std::string(e.what()));
                throw;
            }
        }

        if (system_scan_flag)
        {
            logger.info("Preparing system-wide scan...");
            try
            {
                filesToBeScanned = std::move(support::filesystem_utils::load_files_from_system());
                logger.info("System scan prepared: " + std::to_string(filesToBeScanned.size()) + " files queued");
            }
            catch (const std::exception& e)
            {
                logger.error("Failed to prepare system scan: " + std::string(e.what()));
                throw;
            }
        }

        if (quick_scan_flag)
        {
            logger.info("Applying quick scan filter (modified files only)...");
            try
            {
                auto original_count = filesToBeScanned.size();
                filesToBeScanned = std::move(index_manager::filterModified(filesToBeScanned));
                logger.info("Quick scan filter applied: " + std::to_string(original_count) + " -> " + std::to_string(filesToBeScanned.size()) + " files");
            }
            catch (const std::exception& e)
            {
                logger.warn("Quick scan filter failed, proceeding with full scan: " + std::string(e.what()));
            }
        }

        if (clean_flag)
        {
            performCleaning(OUTPUT_FILE);
            return 0;
        }

        if (!filesToBeScanned.empty())
        {
            processFiles(filesToBeScanned, OUTPUT_FILE, NUMBER_OF_THREADS);
        }
        else
        {
            logger.warn("No files specified for scanning. Use --help for usage information.");
        }

    }
    catch (const BasicErrorBuild& ERROR)
    {
        logger.error("Application error: " + std::string(ERROR.getTips()));
        logger.logAntivirusShutdown();
        throw;
    }
    catch (const CLI::ParseError &error)
    {
        logger.error("Command line parsing error");
        logger.logAntivirusShutdown();
        return app.exit(error);
    }
    catch (const std::exception& e)
    {
        logger.error("Unexpected error: " + std::string(e.what()));
        logger.logAntivirusShutdown();
        throw;
    }

    logger.logAntivirusShutdown();
    return 0;
}

