#include "../include/IndexManager.hpp"
#include "../../ERRORS_PACK/include/errors.hpp"
#include "../../HELPERS/include/Types.hpp"
#include "../../HELPERS/include/support.hpp"
#include "../../HELPERS/include/ThreadPool.hpp"
#include "../include/FileManager.hpp"
#include "../../ConfigManager/include/ConfigManager.hpp"
#include "../../Logger/include/Logger.hpp"
#include <algorithm>
#include <iostream>
#include <vector>
#include <future>

namespace
{
    namespace constants
    {
        const auto INDEX_STORAGE_PATH = config_manager::get_index_storage_path();
        const auto LEGEND_INDEX_FILE = INDEX_STORAGE_PATH + "/Legend.json";
        const auto NUMBER_OF_THREADS = config_manager::get_number_of_threads();
    }

    namespace helper_functions
    {
        template <typename T>
        auto add_property(JSON &json, const std::string &key, const T& value) -> void
        {
            auto& logger = logger::Logger::getInstance();
            logger.debug(std::format("Adding property '{}' to JSON object", key));
            json[key] = value;
        }

        auto create_file_entry(const PATH &file) -> JSON
        {
            auto& logger = logger::Logger::getInstance();
            logger.debug(std::format("Creating file entry for: {}", file.string()));

            JSON entry;

            logger.debug("Calculating file hash...");
            add_property(entry, "Hash", filemanager::file::hash(file));

            logger.debug("Getting modification time...");
            add_property(entry, "Modification time", filemanager::file::lastModificationTime(file));

            logger.debug("Getting file size...");
            add_property(entry, "Size", filemanager::file::size(file));

            logger.debug(std::format("File entry created successfully for: {}", file.string()));
            return entry;
        }

        auto update_json_with_files(JSON &json, const PATHS_CONTAINER &files) -> void
        {
            auto& logger = logger::Logger::getInstance();
            logger.info(std::format("Updating JSON with {} files", files.size()));

            size_t processedCount = 0;
            std::ranges::for_each(files, [&](const PATH &val) -> void {
                logger.debug(std::format("Processing file {}/{}: {}", ++processedCount, files.size(), val.string()));
                json[val.c_str()] = create_file_entry(val);
            });

            logger.info("JSON update completed successfully");
        }

        auto updateFileIndex(const PATH& filePath) -> void
        {
            using namespace constants;

            auto& logger = logger::Logger::getInstance();
            logger.info(std::format("Updating index for file: {}", filePath.string()));

            if (!filemanager::validate::validate_file(filePath))
            {
                logger.error(std::format("File validation failed: {}", filePath.string()));
                throw FileValidationError(filePath.string());
            }

            logger.debug(std::format("Reading legend data from: {}", LEGEND_INDEX_FILE));
            JSON LegendData = support::json_utils::read_data(LEGEND_INDEX_FILE);
            bool already_exists_in_database = false;

            logger.debug("Searching for file in existing indexes...");
            std::ranges::for_each(LegendData, [&](const std::string& legendValue) -> void {
                try {
                    logger.debug(std::format("Checking index: {}", legendValue));
                    JSON ValueData = support::json_utils::read_data(legendValue);

                    if (ValueData.contains(filePath.string())) {
                        logger.info(std::format("File found in index: {}", legendValue));
                        auto& KEY = ValueData[filePath.string()];

                        logger.debug("Updating file hash...");
                        KEY["Hash"] = filemanager::file::hash(filePath);

                        logger.debug("Updating modification time...");
                        KEY["Modification time"] = filemanager::file::lastModificationTime(filePath);

                        logger.debug("Updating file size...");
                        KEY["Size"] = filemanager::file::size(filePath);

                        logger.debug(std::format("Writing updated data to: {}", legendValue));
                        support::json_utils::write_data(legendValue, ValueData);

                        already_exists_in_database = true;
                        logger.success(std::format("File index updated successfully: {}", filePath.string()));
                    }
                } catch (std::exception& ERROR) {
                    logger.error(std::format("Error updating file in index {}: {}", legendValue, ERROR.what()));
                    throw;
                }
            });

            if (!already_exists_in_database) {
                logger.info("File not found in existing indexes, checking parent directory...");

                if (filePath.has_parent_path()) {
                    const auto ParentPathOfNewFile = filePath.parent_path();
                    logger.debug(std::format("Parent path: {}", ParentPathOfNewFile.string()));

                    if (LegendData.contains(ParentPathOfNewFile.string()) && LegendData[ParentPathOfNewFile.string()].is_string()) {
                        logger.info(std::format("Parent directory found in legend: {}", ParentPathOfNewFile.string()));

                        try {
                            auto ParentDirectoryIndex = std::filesystem::path(LegendData[ParentPathOfNewFile.string()].get<std::string>());
                            logger.debug(std::format("Loading parent directory index: {}", ParentDirectoryIndex.string()));

                            JSON PARENT_DIRECTORY = std::move(support::json_utils::read_data(ParentDirectoryIndex));

                            logger.debug("Creating new file entry in parent directory index...");
                            PARENT_DIRECTORY[filePath.string()] = {
                                {"Hash", std::move(filemanager::file::hash(filePath))},
                                {"Modification time", filemanager::file::lastModificationTime(filePath)},
                                {"Size", filemanager::file::size(filePath)}
                            };

                            logger.debug(std::format("Writing updated parent directory index: {}", ParentDirectoryIndex.string()));
                            support::json_utils::write_data(ParentDirectoryIndex, PARENT_DIRECTORY);
                            logger.success(std::format("File added to parent directory index: {}", filePath.string()));

                        } catch (std::exception& ERROR) {
                            logger.error(std::format("Error adding file to parent directory index: {}", ERROR.what()));
                            throw;
                        }
                    } else {
                        logger.warn(std::format("Parent directory not found in legend: {}", ParentPathOfNewFile.string()));
                    }
                }
                else if (LegendData.contains("SYSTEM") && LegendData["SYSTEM"].is_string()) {
                    logger.info("Adding file to SYSTEM index...");
                    const auto SYS_IDX_PATH = std::filesystem::path(LegendData["SYSTEM"].get<std::string>());

                    try {
                        logger.debug(std::format("Loading SYSTEM index: {}", SYS_IDX_PATH.string()));
                        JSON SYSTEM_INDEXES = std::move(support::json_utils::read_data(SYS_IDX_PATH));

                        logger.debug("Creating new file entry in SYSTEM index...");
                        SYSTEM_INDEXES[filePath.string()] = {
                            {"Hash", std::move(filemanager::file::hash(filePath))},
                            {"Modification time", filemanager::file::lastModificationTime(filePath)},
                            {"Size", filemanager::file::size(filePath)}
                        };

                        logger.debug(std::format("Writing updated SYSTEM index: {}", SYS_IDX_PATH.string()));
                        support::json_utils::write_data(SYS_IDX_PATH, SYSTEM_INDEXES);
                        logger.success(std::format("File added to SYSTEM index: {}", filePath.string()));

                    } catch (std::exception& ERROR) {
                        logger.error(std::format("Error adding file to SYSTEM index: {}", ERROR.what()));
                        throw;
                    }
                }
                else {
                    logger.warn(std::format("No suitable index found for file: {}", filePath.string()));
                    logger.warn(std::format("Consider creating new database for directory: {}", filePath.parent_path().string()));
                    logger.warn("Update existing or create SYSTEM indexes storage to fetch data about new files");
                }
            }

            logger.info(std::format("File index update completed: {}", filePath.string()));
        }
    }
}

namespace index_manager
{
    auto createMetaIndex(const std::string &inputPath, const std::string &nameOutputPath) -> void {
        using namespace constants;
        using namespace helper_functions;
        namespace FS = std::filesystem;

        auto& logger = logger::Logger::getInstance();
        logger.info(std::format("Creating new meta index for '{}' with name '{}'...", inputPath, nameOutputPath));

        if (FS::exists(nameOutputPath))
        {
            logger.error(std::format("Directory with name '{}' already exists! Consider using another name", nameOutputPath));
            throw OverwritingOtherFileError();
        }

        JSON filesIndexLegend;
        try
        {
            logger.debug(std::format("Reading legend data from: {}", LEGEND_INDEX_FILE));
            filesIndexLegend = support::json_utils::read_data(LEGEND_INDEX_FILE);
        }
        catch (const std::exception& e)
        {
            logger.error(std::format("Error reading data from Legend.json: {}", e.what()));
            throw;
        }

        logger.info(std::format("Successfully read data about indexes from: {}", LEGEND_INDEX_FILE));
        auto ABSDirPath = std::filesystem::absolute(std::filesystem::path(inputPath));
        logger.debug(std::format("Absolute path: {}", ABSDirPath.string()));

        if (filesIndexLegend.contains(ABSDirPath))
        {
            logger.warn(std::format("Meta index for '{}' already exists", inputPath));
            logger.warn(std::format("Data about files from this directory are stored in: {}",
                support::json_utils::read_data(LEGEND_INDEX_FILE)[inputPath].get<std::string>()));
            logger.logAntivirusShutdown();
            return;
        }

        std::string indexFileName = std::move(const_cast<std::string &>(nameOutputPath)) + ".json";
        indexFileName = INDEX_STORAGE_PATH + "/IndexFiles/" + indexFileName;
        logger.debug(std::format("Index file will be created at: {}", indexFileName));

        JSON indexJsonData;

        if (inputPath != "SYSTEM")
        {
            logger.info(std::format("Loading files from directory: {}", ABSDirPath.string()));
            auto files = support::filesystem_utils::load_from_directory(ABSDirPath);
            logger.debug(std::format("Found {} files to index", files.size()));
            update_json_with_files(indexJsonData, files);
            filesIndexLegend[ABSDirPath] = indexFileName;
        }
        else
        {
            logger.info("Loading system files...");
            auto files = support::filesystem_utils::load_files_from_system();
            logger.debug(std::format("Found {} system files to index", files.size()));
            update_json_with_files(indexJsonData, files);
            filesIndexLegend["SYSTEM"] = indexFileName;
        }
        logger.info("Successfully prepared new metaindex data");

        try
        {
            logger.debug(std::format("Saving legend data to: {}", LEGEND_INDEX_FILE));
            support::json_utils::write_data(LEGEND_INDEX_FILE, filesIndexLegend);

            logger.debug(std::format("Saving index data to: {}", indexFileName));
            support::json_utils::write_data(indexFileName, indexJsonData);
        }
        catch (const std::exception& ERROR)
        {
            logger.error(std::format("Error occurred while saving data about new meta index: {}", ERROR.what()));
            throw;
        }
        logger.success(std::format("Successfully created new meta index for '{}'!", inputPath));
        logger.info(std::format("Check details in {}", LEGEND_INDEX_FILE));
    }

    auto updateMetaIndex(const std::string &inputPath) -> void {
        using namespace constants;
        using namespace helper_functions;

        auto& logger = logger::Logger::getInstance();
        logger.info(std::format("Updating meta index for: {}", inputPath));

        JSON filesIndexLegend;
        try
        {
            logger.debug(std::format("Reading legend data from: {}", LEGEND_INDEX_FILE));
            filesIndexLegend = support::json_utils::read_data(LEGEND_INDEX_FILE);
        }
        catch (const std::exception& ERROR)
        {
            logger.error(std::format("Error reading data from {}: {}", LEGEND_INDEX_FILE, ERROR.what()));
            throw;
        }
        logger.info(std::format("Successfully read data from {}", LEGEND_INDEX_FILE));

        if (!filesIndexLegend.contains(inputPath))
        {
            logger.error(std::format("Provided meta index does not exist in database: {}", inputPath));
            throw MetaIndexDoNotExists();
        }

        auto foundIndexPath = std::move(filesIndexLegend[inputPath]);
        logger.info(std::format("Found index path: {}", foundIndexPath.get<std::string>()));

        JSON updatedIndex;

        logger.debug("Updating index data...");
        if (inputPath != "SYSTEM")
        {
            logger.info(std::format("Loading files from directory: {}", inputPath));
            auto files = support::filesystem_utils::load_from_directory(inputPath);
            logger.debug(std::format("Found {} files to index", files.size()));
            update_json_with_files(updatedIndex, files);
        }
        else
        {
            logger.info("Loading system files...");
            auto files = support::filesystem_utils::load_files_from_system();
            logger.debug(std::format("Found {} system files to index", files.size()));
            update_json_with_files(updatedIndex, files);
        }

        try
        {
            logger.debug(std::format("Saving updated index to: {}", foundIndexPath.get<std::string>()));
            support::json_utils::write_data(foundIndexPath, updatedIndex);
        }
        catch (const std::exception& ERROR)
        {
            logger.error(std::format("Error occurred while saving data: {}", ERROR.what()));
            throw;
        }
        logger.success(std::format("Successfully updated meta index for: {}", inputPath));
    }

    auto filterModified(PATHS_CONTAINER &files_input_ref) -> PATHS_CONTAINER {
        using namespace constants;
        using namespace helper_functions;
        namespace FS = std::filesystem;

        auto& logger = logger::Logger::getInstance();
        logger.info(std::format("Filtering modified files from {} input files", files_input_ref.size()));

        if (!filemanager::validate::validate_directory(INDEX_STORAGE_PATH))
        {
            logger.error(std::format("Error: Index storage path is not a valid directory: {}", INDEX_STORAGE_PATH));
            throw DirectoryValidationError(INDEX_STORAGE_PATH);
        }

        if (files_input_ref.empty())
        {
            logger.info("Input file list is empty, nothing to filter");
            return {};
        }

        JSON SystemIndexData;
        bool systemIndexLoadedSuccessfully = false;

        try
        {
            logger.debug("Attempting to load SYSTEM index for filtering...");
            if (std::filesystem::exists(LEGEND_INDEX_FILE)) {
                logger.debug(std::format("Reading legend file: {}", LEGEND_INDEX_FILE));
                JSON indexJsonLegend = support::json_utils::read_data(LEGEND_INDEX_FILE);

                if (indexJsonLegend.contains("SYSTEM") && indexJsonLegend["SYSTEM"].is_string()) {
                    std::string systemIndexPathStr = indexJsonLegend["SYSTEM"].get<std::string>();
                    logger.debug(std::format("SYSTEM index path: {}", systemIndexPathStr));

                    if (!systemIndexPathStr.empty() && std::filesystem::exists(systemIndexPathStr)) {
                        logger.info(std::format("Loading SYSTEM index from: {}", systemIndexPathStr));
                        SystemIndexData = support::json_utils::read_data(std::filesystem::path(systemIndexPathStr));

                        if (!SystemIndexData.empty()) {
                            logger.debug(std::format("SYSTEM index loaded successfully with {} entries", SystemIndexData.size()));
                            systemIndexLoadedSuccessfully = true;
                        } else {
                            logger.warn("SYSTEM index is empty. All input files will be kept.");
                        }
                    } else {
                        logger.warn(std::format("SYSTEM index path '{}' is invalid or file does not exist. Cannot filter.", systemIndexPathStr));
                    }
                } else {
                    logger.warn("'SYSTEM' entry not found or not a string in Legend.json. Cannot filter using SYSTEM index.");
                }
            } else {
                logger.warn(std::format("Index Legend file does not exist at {}. Cannot filter using SYSTEM index.", LEGEND_INDEX_FILE));
            }
        }
        catch (const std::exception& e) {
            logger.error(std::format("Error during SYSTEM index data loading: {}. All files will be treated as new/modified.", e.what()));
        }

        if (!systemIndexLoadedSuccessfully) {
            logger.warn("SYSTEM index could not be loaded or is empty. All input files will be kept.");
            return files_input_ref;
        }

        logger.info(std::format("Starting file modification check using {} threads", NUMBER_OF_THREADS));
        ThreadPool threadPool(NUMBER_OF_THREADS);
        std::vector<std::future<bool>> taskFutures;
        taskFutures.reserve(files_input_ref.size());

        std::mutex system_index_data_mutex;

        auto check_file_task = [&SystemIndexData, &system_index_data_mutex, &logger](const PATH &filePath) -> bool {
            try {
                auto absPATH_str = std::filesystem::absolute(filePath).string();
                std::lock_guard<std::mutex> lock(system_index_data_mutex);

                if (SystemIndexData.contains(absPATH_str)) {
                    const auto &file_entry = SystemIndexData.at(absPATH_str);

                    if (file_entry.contains("Modification time") && file_entry.contains("Size") && file_entry.contains("Hash")) {
                        auto lastModTime = file_entry["Modification time"].get<time_t>();
                        auto lastSize = file_entry["Size"].get<ssize_t>();
                        auto lastHash = file_entry["Hash"].get<std::string>();

                        if (filemanager::file::isMod(filePath, lastModTime, lastSize, lastHash)) {
                            logger.debug(std::format("File is modified: {}", filePath.string()));
                            return false;
                        }
                        logger.debug(std::format("File is unchanged: {}", filePath.string()));
                        return true;
                    }
                    logger.warn(std::format("Malformed index entry for {} (missing fields). Keeping file.", absPATH_str));
                    return false;
                }
                logger.debug(std::format("New file (not in index): {}", filePath.string()));
                return false;
            } catch (const nlohmann::json::exception &e) {
                logger.warn(std::format("JSON exception for {} against SYSTEM index: {}. Keeping file.", filePath.string(), e.what()));
                return false;
            } catch (const FS::filesystem_error& efs) {
                logger.warn(std::format("Filesystem error for {}: {}. Keeping file.", filePath.string(), efs.what()));
                return false;
            } catch (const std::exception &e) {
                logger.warn(std::format("Standard exception for {} against SYSTEM index: {}. Keeping file.", filePath.string(), e.what()));
                return false;
            } catch (...) {
                logger.warn(std::format("Unknown exception for {} against SYSTEM index. Keeping file.", filePath.string()));
                return false;
            }
        };

        logger.debug("Submitting file check tasks to thread pool...");
        for (const auto& filePath : files_input_ref) {
            taskFutures.emplace_back(threadPool.addTask(check_file_task, filePath));
        }

        logger.info("Collecting results from file check tasks...");
        PATHS_CONTAINER filesToKeep;
        filesToKeep.reserve(files_input_ref.size());

        for (size_t i = 0; i < taskFutures.size(); ++i) {
            bool should_remove = true;
            try {
                should_remove = taskFutures.at(i).get();
            } catch (const std::exception& e) {
                logger.warn(std::format("Exception from task future for file {}: {}. Keeping file.",
                          (i < files_input_ref.size() ? files_input_ref.at(i).string() : "unknown_file_due_to_index_mismatch"),
                          e.what()));
                should_remove = false;
            }

            if (!should_remove) {
                if (i < files_input_ref.size()) {
                    filesToKeep.push_back(files_input_ref.at(i));
                } else {
                    logger.error(std::format("Error: Index mismatch between task futures and input files. Cannot keep file for future at index {}", i));
                }
            }
        }

        logger.info(std::format("Filter results: {} out of {} files kept for scanning (modified or new)", filesToKeep.size(), files_input_ref.size()));

        if (!filesToKeep.empty()) {
            logger.info("Updating indexes for modified files...");
            std::ranges::for_each(filesToKeep, [&](const PATH& filePath) -> void {
                try {
                    updateFileIndex(filePath);
                } catch (const std::exception& e) {
                    logger.error(std::format("Failed to update index for {}: {}", filePath.string(), e.what()));
                }
            });
        }

        logger.success("File filtering completed successfully");
        return filesToKeep;
    }

    auto updateAfterRemoval(const PATH& filePath) -> void
    {
        namespace FS = std::filesystem;
        using namespace constants;

        auto& logger = logger::Logger::getInstance();
        logger.info(std::format("Updating indexes after file removal: {}", filePath.string()));

        if (!filemanager::validate::validate_file(filePath))
        {
            logger.error(std::format("File validation failed: {}", filePath.string()));
            throw FileValidationError(filePath.string());
        }

        JSON LegendData;
        try
        {
            logger.debug(std::format("Reading legend data from: {}", LEGEND_INDEX_FILE));
            LegendData = support::json_utils::read_data(LEGEND_INDEX_FILE);
        }
        catch (const std::exception& e) {
            logger.error(std::format("Error reading Legend data from Index Database: {}", e.what()));
            throw;
        }

        logger.debug("Scanning indexes for file references...");
        bool fileFound = false;
        std::ranges::for_each(LegendData, [&](const std::string& LegendValue) -> void
        {
            logger.debug(std::format("Checking index: {}", LegendValue));
            JSON SingleIndexStorage;

            try
            {
                SingleIndexStorage = support::json_utils::read_data(LegendValue);
            }
            catch (const std::exception& e) {
                logger.error(std::format("Error reading data from provided Index Storage Path {}: {}", LegendValue, e.what()));
                throw;
            }

            if (SingleIndexStorage.contains(filePath.string()))
            {
                logger.info(std::format("File found in index: {}", LegendValue));
                fileFound = true;

                logger.debug(std::format("Removing file entry from index: {}", filePath.string()));
                SingleIndexStorage.erase(filePath.string());

                try
                {
                    logger.debug(std::format("Writing updated index data to: {}", LegendValue));
                    support::json_utils::write_data(std::filesystem::path(LegendValue), SingleIndexStorage);
                    logger.success(std::format("Successfully removed file entry from index: {}", filePath.string()));
                }
                catch (const std::exception& e)
                {
                    logger.error(std::format("Error writing Index data after file removal: {}", e.what()));
                    logger.error(std::format("Problem with: {} index storage and {}", LegendValue, filePath.string()));
                    throw;
                }
            }
        });

        if (!fileFound) {
            logger.warn(std::format("File not found in any index: {}", filePath.string()));
        }

        logger.info(std::format("Index update after file removal completed: {}", filePath.string()));
    }
}