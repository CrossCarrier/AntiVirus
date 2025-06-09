#include "../include/ConfigManager.hpp"
#include "../../HELPERS/include/support.hpp"
#include "../../FileManager/include/FileManager.hpp"
#include "../../ERRORS_PACK/include/errors.hpp"
#include "../../HELPERS/include/Types.hpp"
#include "../../Logger/include/Logger.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>
#include <format>

namespace {
    using namespace types::filesystem_types;
    using namespace types::json;

    namespace constants
    {
        constexpr auto USER_SETTINGS_PATH = "../antivirus/AppData/user_settings.json";

        constexpr auto FILES_KEY = "Files";
        constexpr auto DIRECTORIES_KEY = "Directories";

        constexpr auto JSON_KEY_INDEXES = "Indexes storage path";
        constexpr auto JSON_KEY_THREADS_NUMBER = "Number of threads";
        constexpr auto JSON_KEY_OUTPUT_FILE = "Output file path";
    }

    namespace helper_functions
    {}

    namespace helper_functions::set_index_storage_locations_HELPERS
    {
        auto ValidationOnEntry(const PATH& newIndexStorageLocation) -> void
        {
            namespace FS = std::filesystem;
            auto& logger = logger::Logger::getInstance();

            logger.debug(std::format("Validating index storage location: {}", newIndexStorageLocation.string()));

            if (!FS::exists(newIndexStorageLocation))
            {
                logger.debug(std::format("Directory does not exist, attempting to create: {}", newIndexStorageLocation.string()));
                try
                {
                    if (!FS::create_directories(newIndexStorageLocation))
                    {
                        if (FS::exists(newIndexStorageLocation) && !FS::is_directory(newIndexStorageLocation))
                        {
                            logger.error(std::format("Path exists but is not a directory: {}", newIndexStorageLocation.string()));
                            throw DirectoryCreationError("Path '" + newIndexStorageLocation.string() + "' exists and is not a directory.");
                        }
                        logger.error(std::format("Failed to create directory: {}", newIndexStorageLocation.string()));
                        throw DirectoryCreationError("Failed to create directory '" + newIndexStorageLocation.string());
                    }
                    logger.debug(std::format("Directory created successfully: {}", newIndexStorageLocation.string()));
                }
                catch (const FS::filesystem_error& FILESYSTEM_ERROR)
                {
                    logger.error(std::format("Filesystem error while creating directory: {} - {}",
                        newIndexStorageLocation.string(), FILESYSTEM_ERROR.what()));
                    throw DirectoryCreationError("Failed to create directory '" + newIndexStorageLocation.string() + "': " + FILESYSTEM_ERROR.what());
                }
            }
            else if (!FS::is_directory(newIndexStorageLocation))
            {
                logger.error(std::format("Path exists but is not a directory: {}", newIndexStorageLocation.string()));
                throw DirectoryCreationError("Path '" + newIndexStorageLocation.string() + "' exists but is not a directory.");
            }

            logger.debug(std::format("Index storage location validation successful: {}", newIndexStorageLocation.string()));
        }

        auto CopyDirectoryContent(const JSON& userSettings, const PATH& from, const PATH& destination) -> void
        {
            namespace FS = std::filesystem;
            using namespace constants;
            auto& logger = logger::Logger::getInstance();

            logger.info(std::format("Copying index data from '{}' to '{}'", from.string(), destination.string()));
            constexpr auto copyOptions = FS::copy_options::recursive | FS::copy_options::overwrite_existing;

            if (userSettings.contains(JSON_KEY_INDEXES) && userSettings[JSON_KEY_INDEXES].is_string())
            {
                if (FS::exists(from))
                {
                    if (filemanager::validate::validate_directory(from))
                    {
                        if (FS::absolute(from) != FS::absolute(destination))
                        {
                            try
                            {
                                logger.debug(std::format("Starting directory copy operation..."));
                                FS::copy(from, destination, copyOptions);
                                logger.success(std::format("Directory contents copied successfully from '{}' to '{}'",
                                    from.string(), destination.string()));
                            }
                            catch (const FS::filesystem_error& FILESYSTEM_ERROR)
                            {
                                logger.error(std::format("Filesystem error during copy operation: {}", FILESYSTEM_ERROR.what()));
                                throw FilesystemOperationError("Failed to copy from '" + from.string() + "' to '" + destination.string() + "': " + FILESYSTEM_ERROR.what());
                            }
                        }
                        else {
                            logger.debug("Source and destination are the same, skipping copy");
                        }
                    }
                    else
                    {
                        logger.error(std::format("Invalid directory structure at source location: {}", from.string()));
                        throw MovingIndexStorageError("Invalid old indexes storage : " + from.string());
                    }
                }
                else {
                    logger.debug(std::format("Source directory does not exist, nothing to copy: {}", from.string()));
                }
            }
            else
            {
                logger.error("User settings missing 'Indexes storage path' field or it has incorrect type");
                throw ConfigFileStructureError("Index field error");
            }
        }

        auto RemoveOldContent(const PATH& oldIndexStorage, const PATH& newIndexStorage) -> void
        {
            namespace FS = std::filesystem;
            auto& logger = logger::Logger::getInstance();

            logger.info(std::format("Removing old index storage content: {}", oldIndexStorage.string()));

            if (FS::is_directory(oldIndexStorage) && (FS::absolute(oldIndexStorage) != FS::absolute(newIndexStorage)))
            {
                try
                {
                    logger.debug("Starting directory removal...");
                    FS::remove_all(oldIndexStorage);
                    if (FS::exists(oldIndexStorage))
                    {
                        logger.error(std::format("Failed to remove old directory: {}", oldIndexStorage.string()));
                        throw RemovingDirectoryContentError("Failed to remove old directory '" + oldIndexStorage.string() + "' (still exists after remove_all).");
                    }
                    logger.success(std::format("Old directory removed successfully: {}", oldIndexStorage.string()));
                }
                catch (const FS::filesystem_error& FILESYSTEM_ERROR)
                {
                    logger.error(std::format("Filesystem error during directory removal: {}", FILESYSTEM_ERROR.what()));
                    throw RemovingDirectoryContentError("Failed to remove old directory content : " + oldIndexStorage.string() + "': " + FILESYSTEM_ERROR.what());
                }
            }
            else {
                logger.debug("Old and new directories are the same or old directory is not valid, skipping removal");
            }
        }

        auto UpdateNewIndexStorage(const PATH& newIndexStorage) -> void
        {
            namespace FS = std::filesystem;
            auto& logger = logger::Logger::getInstance();

            logger.info(std::format("Updating index paths in new storage location: {}", newIndexStorage.string()));

            try
            {
                const FS::path newLegendIndexPath = newIndexStorage / "Legend.json";
                logger.debug(std::format("Legend.json path: {}", newLegendIndexPath.string()));

                if (FS::exists(newLegendIndexPath))
                {
                    JSON legendJsonData;
                    try
                    {
                        logger.debug("Reading Legend.json data...");
                        legendJsonData = support::json_utils::read_data(newLegendIndexPath);
                    }
                    catch (const std::exception& READING_JSON_ERROR)
                    {
                        logger.error(std::format("Error reading Legend.json data: {}", READING_JSON_ERROR.what()));
                        throw;
                    }

                    bool legend_was_modified = false;
                    logger.debug("Checking and updating index file paths...");
                    std::ranges::for_each(legendJsonData.items(), [&](const auto& item) -> void
                    {
                        if (item.value().is_string())
                        {
                            auto current_index_file_path_str = item.value().template get<std::string>();
                            const PATH current_index_file_path(current_index_file_path_str);
                            const PATH index_file_name = current_index_file_path.filename();

                            const PATH new_full_index_file_path = newIndexStorage / "IndexFiles" / index_file_name;
                            if (current_index_file_path_str != new_full_index_file_path.string())
                            {
                                logger.debug(std::format("Updating path: {} -> {}",
                                    current_index_file_path_str, new_full_index_file_path.string()));
                                item.value() = new_full_index_file_path.string();
                                legend_was_modified = true;
                            }
                        }
                    });

                    if (legend_was_modified)
                    {
                        logger.debug("Writing updated Legend.json data...");
                        support::json_utils::write_data(newLegendIndexPath.string(), legendJsonData);
                        logger.success("Legend.json updated successfully with new paths");
                    }
                    else {
                        logger.debug("No path updates needed in Legend.json");
                    }
                }
                else
                {
                    logger.warn(std::format("Legend.json not found at {}. Cannot update index paths.", newLegendIndexPath.string()));
                }
            }
            catch (const FS::filesystem_error& FILESYSTEM_ERROR)
            {
                logger.error(std::format("Filesystem error during index update: {}", FILESYSTEM_ERROR.what()));
                throw FilesystemOperationError(FILESYSTEM_ERROR.what());
            }
            catch (const std::exception& ERROR)
            {
                logger.error(std::format("Error updating index storage: {}", ERROR.what()));
                throw;
            }
        }
    }

    namespace helper_functions::getters_HELPERS
    {
        auto fetchUserSetting(std::string&& JSON_KEY, std::string&& USER_SETTINGS_PATH) -> nlohmann::basic_json<>
        {
            auto& logger = logger::Logger::getInstance();
            logger.debug(std::format("Fetching user setting: '{}' from '{}'", JSON_KEY, USER_SETTINGS_PATH));

            JSON userSettings;
            try
            {
                userSettings = support::json_utils::read_data(USER_SETTINGS_PATH);
                logger.debug("User settings loaded successfully");
            }
            catch (std::exception& ERROR)
            {
                logger.error(std::format("Error reading user settings: {}", ERROR.what()));
                throw;
            }

            if (userSettings.contains(JSON_KEY))
            {
                logger.debug(std::format("Setting '{}' found with value: {}", JSON_KEY, userSettings[JSON_KEY].dump()));
                return userSettings[JSON_KEY];
            }

            logger.error(std::format("Setting '{}' not found in user settings", JSON_KEY));
            throw ReadingSettingsError("KeyError");
        }
    }

    namespace helper_functions::set_output_file_HELPERS
    {
        auto UpdateOutputFileField (JSON& userSettings, const PATH& newLocation) -> void
        {
            using namespace constants;
            auto& logger = logger::Logger::getInstance();

            logger.debug(std::format("Updating output file setting to: {}", newLocation.string()));

            if (userSettings.contains(JSON_KEY_OUTPUT_FILE) && userSettings[JSON_KEY_OUTPUT_FILE].is_string())
            {
                try
                {
                    userSettings[JSON_KEY_OUTPUT_FILE] = newLocation.string();
                    logger.debug("Writing updated user settings...");
                    support::json_utils::write_data(USER_SETTINGS_PATH, userSettings);
                    logger.debug("User settings updated successfully");
                }
                catch (const std::exception& ERROR)
                {
                    logger.error(std::format("Error updating output file setting: {}", ERROR.what()));
                    throw UpdatingUserSettingsError("Output file");
                }
            }
            else
            {
                logger.error("Output file path field missing or has incorrect type in user settings");
                throw ConfigFileStructureError("Output file path field cannot be detected!");
            }
        }
    }

    namespace helper_functions::change_number_of_threads_HELPERS
    {
        auto ValidateNumberOfThreads(const int providedNumberOfThreads) -> bool
        {
            auto& logger = logger::Logger::getInstance();

            bool isValid = (providedNumberOfThreads > 0 && providedNumberOfThreads <= std::thread::hardware_concurrency());
            logger.debug(std::format("Thread count validation: {} (valid: {}, max hardware: {})",
                providedNumberOfThreads, isValid ? "yes" : "no", std::thread::hardware_concurrency()));

            return isValid;
        }

        auto UpdateThreadsField(JSON& userSettings, const int newNumberOfThreads) -> void
        {
            using namespace constants;
            auto& logger = logger::Logger::getInstance();

            logger.debug(std::format("Updating thread count setting to: {}", newNumberOfThreads));

            if (userSettings.contains(JSON_KEY_THREADS_NUMBER) && userSettings[JSON_KEY_THREADS_NUMBER].is_number_unsigned())
            {
                try
                {
                    userSettings[JSON_KEY_THREADS_NUMBER] = newNumberOfThreads;
                    logger.debug("Writing updated user settings...");
                    support::json_utils::write_data(USER_SETTINGS_PATH, userSettings);
                    logger.debug("User settings updated successfully");
                }
                catch (const std::exception& ERROR)
                {
                    logger.error(std::format("Error updating thread count setting: {}", ERROR.what()));
                    throw UpdatingUserSettingsError("Threads");
                }
            }
            else
            {
                logger.error("Thread count field missing or has incorrect type in user settings");
                throw ConfigFileStructureError("Number of threads filed cannot be detected!");
            }
        }
    }
}


namespace config_manager {

    auto get_number_of_threads() -> int {
        using namespace constants;
        using namespace helper_functions::getters_HELPERS;

        auto& logger = logger::Logger::getInstance();
        logger.debug("Getting number of threads from user settings");

        try
        {
            auto threads = fetchUserSetting(JSON_KEY_THREADS_NUMBER, USER_SETTINGS_PATH);
            logger.debug(std::format("Thread count value retrieved: {}", threads.get<int>()));
            return threads;
        }
        catch (const std::exception& ERROR)
        {
            logger.error(std::format("Failed to get thread count: {}", ERROR.what()));
            throw;
        }
    }

    auto get_index_storage_path() -> std::string {
        using namespace constants;
        using namespace helper_functions::getters_HELPERS;

        auto& logger = logger::Logger::getInstance();
        logger.debug("Getting index storage path from user settings");

        try
        {
            auto path = fetchUserSetting(JSON_KEY_INDEXES, USER_SETTINGS_PATH);
            logger.debug(std::format("Index storage path retrieved: {}", path.get<std::string>()));
            return path;
        }
        catch (const std::exception& ERROR)
        {
            logger.error(std::format("Failed to get index storage path: {}", ERROR.what()));
            throw;
        }
    }

    auto get_output_file_path() -> std::string {
        using namespace constants;
        using namespace helper_functions::getters_HELPERS;

        auto& logger = logger::Logger::getInstance();
        logger.debug("Getting output file path from user settings");

        try
        {
            auto path = fetchUserSetting(JSON_KEY_OUTPUT_FILE, USER_SETTINGS_PATH);
            logger.debug(std::format("Output file path retrieved: {}", path.get<std::string>()));
            return path;
        }
        catch (const std::exception& ERROR)
        {
            logger.error(std::format("Failed to get output file path: {}", ERROR.what()));
            throw;
        }
    }


    auto fetch_config_files(const PATH &config_file_path) -> CONFIG_FILES {
        using namespace constants;
        namespace FS = std::filesystem;

        auto& logger = logger::Logger::getInstance();
        logger.info(std::format("Fetching files from config file: {}", config_file_path.string()));

        UNIQUE_CONFIG_FILES fetched_files;
        CONFIG_FILES unique_fetched_files;
        JSON config_data;

        try
        {
            logger.debug("Reading config file data...");
            config_data = support::json_utils::read_data(config_file_path);
            logger.debug("Config file loaded successfully");
        }
        catch (std::exception& ERROR)
        {
            logger.error(std::format("Error reading config file: {}", ERROR.what()));
            throw;
        }

        if (config_data.contains(DIRECTORIES_KEY) && config_data[DIRECTORIES_KEY].is_array())
        {
            const auto directoryFieldContent = config_data[DIRECTORIES_KEY].get<std::vector<std::string>>();
            logger.info(std::format("Processing {} directories from config", directoryFieldContent.size()));

            std::ranges::for_each(directoryFieldContent, [&fetched_files, &logger](const std::string &directory_path) -> void {
                    const auto DirPathABS = FS::absolute(directory_path);
                    logger.debug(std::format("Processing directory: {}", DirPathABS.string()));

                    if (!filemanager::validate::validate_directory(DirPathABS))
                    {
                        logger.error(std::format("Invalid directory: {}", DirPathABS.string()));
                        throw DirectoryValidationError("Invalid directory in config: " + DirPathABS.string());
                    }

                    constexpr auto directoryIteratorOptions = ITER_OPTIONS::skip_permission_denied;
                    const auto directoryIteratorR = DIRECTORY_ITER_R(DirPathABS, directoryIteratorOptions);
                    logger.debug("Directory validation successful, scanning for files...");

                    size_t fileCount = 0;
                    std::ranges::for_each(directoryIteratorR,[&fetched_files, &fileCount, &logger](const auto& entry) -> void
                    {
                            if (!entry.is_regular_file()) return; // Skip non regular file

                            if (!filemanager::validate::validate_file(entry.path()))
                            {
                                logger.error(std::format("Invalid file: {}", entry.path().string()));
                                throw FileValidationError("Invalid file in config scan: " + entry.path().string());
                            }

                            fileCount++;
                            fetched_files.insert(FS::absolute(entry.path()));
                    });
                    logger.debug(std::format("Found {} files in directory {}", fileCount, DirPathABS.string()));
            });
        }
        else
        {
            logger.error("Missing or invalid 'Directories' field in config file");
            throw ConfigFileStructureError("Configuration missing 'Directories' field or it's not an array.");
        }

        if (config_data.contains(FILES_KEY) && config_data[FILES_KEY].is_array())
        {
            const auto fileFieldContent = config_data[FILES_KEY].get<std::vector<std::string>>();
            logger.info(std::format("Processing {} individual files from config", fileFieldContent.size()));

            std::ranges::for_each(fileFieldContent, [&fetched_files, &logger](const std::string &file_path) -> void
            {
                const auto FilePathABS = FS::absolute(file_path);
                logger.debug(std::format("Processing file: {}", FilePathABS.string()));

                if (!filemanager::validate::validate_file(FilePathABS))
                {
                    logger.error(std::format("Invalid file: {}", FilePathABS.string()));
                    throw FileValidationError("Invalid file in config: " + FilePathABS.string());
                }
                fetched_files.insert(FilePathABS);
            });
        }
        else
        {
            logger.error("Missing or invalid 'Files' field in config file");
            throw ConfigFileStructureError("Configuration missing 'Files' field or it's not an array.");
        }

        unique_fetched_files.assign(fetched_files.begin(), fetched_files.end());
        logger.success(std::format("Successfully fetched {} unique files from config", unique_fetched_files.size()));
        return unique_fetched_files;
    }

    auto set_index_storage_location(const PATH &new_location) -> void {
        namespace FS = std::filesystem;
        using namespace helper_functions::set_index_storage_locations_HELPERS;
        using namespace constants;

        auto& logger = logger::Logger::getInstance();
        logger.info(std::format("Setting index storage location to: {}", new_location.string()));

        try
        {
            logger.debug("Validating new index storage location...");
            ValidationOnEntry(new_location);
        }
        catch (const DirectoryCreationError& DIRECTORY_CREATION_ERROR)
        {
            logger.error(std::format("Directory creation error: {}", DIRECTORY_CREATION_ERROR.what()));
            throw;
        }

        JSON userSettings;

        try
        {
            logger.debug(std::format("Reading user settings from: {}", USER_SETTINGS_PATH));
            userSettings = support::json_utils::read_data(USER_SETTINGS_PATH);
        }
        catch (const std::exception& ERROR)
        {
            logger.error(std::format("Error reading user settings: {}", ERROR.what()));
            throw ReadingSettingsError("Reading from settings file");
        }

        const auto OLD_INDEXES_DIR_STR = get_index_storage_path();
        const auto OLD_INDEXES_DIR_PATH = std::filesystem::path(OLD_INDEXES_DIR_STR);
        logger.debug(std::format("Current index storage path: {}", OLD_INDEXES_DIR_STR));

        try
        {
            logger.debug("Copying index data from old location to new location...");
            CopyDirectoryContent(userSettings, OLD_INDEXES_DIR_PATH, new_location);
        }
        catch (const std::exception& ERROR)
        {
            logger.error(std::format("Error copying index data: {}", ERROR.what()));
            throw;
        }

        try
        {
            logger.debug("Removing old index storage...");
            RemoveOldContent(OLD_INDEXES_DIR_PATH, new_location);
        }
        catch (const std::exception& ERROR)
        {
            logger.error(std::format("Error removing old index storage: {}", ERROR.what()));
            throw;
        }

        try
        {
            logger.debug("Updating index paths in new storage location...");
            UpdateNewIndexStorage(new_location);
        }
        catch (const std::exception& ERROR)
        {
            logger.error(std::format("Error updating index paths: {}", ERROR.what()));
            throw;
        }

        try
        {
            logger.debug("Updating user settings with new index storage path...");
            userSettings[JSON_KEY_INDEXES] = new_location.string();
            support::json_utils::write_data(USER_SETTINGS_PATH, userSettings);
            logger.success(std::format("Index storage location successfully updated to: {}", new_location.string()));
        }
        catch (const std::exception& ERROR)
        {
            logger.error(std::format("Error updating user settings: {}", ERROR.what()));
            throw UpdatingUserSettingsError("Indexes storage");
         }
    }

    auto set_output_file(const PATH &new_location) -> void
    {
        namespace FS = std::filesystem;
        using namespace helper_functions::set_output_file_HELPERS;
        using namespace constants;

        auto& logger = logger::Logger::getInstance();
        logger.info(std::format("Setting output file to: {}", new_location.string()));

        if (new_location.extension() != ".json")
        {
            logger.error(std::format("Invalid file extension: {}, expected .json", new_location.extension().string()));
            throw InvalidJsonFileFormatError();
        }

        if (new_location.has_parent_path())
        {
            const PATH parent_dir = new_location.parent_path();
            logger.debug(std::format("Checking parent directory: {}", parent_dir.string()));

            if (!FS::exists(parent_dir))
            {
                try
                {
                    logger.debug(std::format("Creating parent directory: {}", parent_dir.string()));
                    FS::create_directories(parent_dir);
                }
                catch (const FS::filesystem_error& FILESYSTEM_ERROR)
                {
                    logger.error(std::format("Failed to create parent directory: {}", FILESYSTEM_ERROR.what()));
                    throw DirectoryCreationError("Failed to create parent directory for output file '" + parent_dir.string() + "': " + FILESYSTEM_ERROR.what());
                }
            }
            else if (!FS::is_directory(parent_dir))
            {
                logger.error(std::format("Parent path is not a directory: {}", parent_dir.string()));
                throw DirectoryValidationError("Parent path for output file '" + parent_dir.string() + "' is not a directory.");
            }
        }

        if (!FS::exists(new_location))
        {
            logger.debug(std::format("Output file does not exist, creating: {}", new_location.string()));
            std::ofstream output_stream_test(new_location.string());
            if (!output_stream_test)
            {
                logger.error(std::format("Failed to create output file: {}", new_location.string()));
                throw StreamOpeningError();
            }
            output_stream_test.close();
        }

        JSON userSettings;
        try
        {
            logger.debug("Reading user settings...");
            userSettings = support::json_utils::read_data(USER_SETTINGS_PATH);
        }
        catch (const std::exception& ERROR)
        {
            logger.error(std::format("Error reading user settings: {}", ERROR.what()));
            throw ReadingSettingsError("Reading from settings file");
        }

        try
        {
            logger.debug("Updating output file setting...");
            UpdateOutputFileField(userSettings, new_location);
            logger.success(std::format("Output file path successfully updated to: {}", new_location.string()));
        }
        catch (const std::exception& ERROR)
        {
            logger.error(std::format("Error updating output file setting: {}", ERROR.what()));
            throw;
        }
    }

    auto change_number_of_threads(int newNumberOfThreads) -> void
    {
        using namespace constants;
        using namespace helper_functions::change_number_of_threads_HELPERS;

        auto& logger = logger::Logger::getInstance();
        logger.info(std::format("Setting number of threads to: {}", newNumberOfThreads));

        if (!ValidateNumberOfThreads(newNumberOfThreads))
        {
            logger.error(std::format("Invalid thread count: {} (must be between 1 and {})",
                newNumberOfThreads, std::thread::hardware_concurrency()));
            throw InvalidNumbersOfThreadsError(newNumberOfThreads);
        }

        JSON userSettings;
        try
        {
            logger.debug("Reading user settings...");
            userSettings = support::json_utils::read_data(USER_SETTINGS_PATH);
        }
        catch (const std::exception& ERROR)
        {
            logger.error(std::format("Error reading user settings: {}", ERROR.what()));
            throw ReadingSettingsError("Threads field");
        }

        try
        {
            logger.debug("Updating thread count setting...");
            UpdateThreadsField(userSettings, newNumberOfThreads);
            logger.success(std::format("Thread count successfully updated to: {}", newNumberOfThreads));
        }
        catch (std::exception& ERROR)
        {
            logger.error(std::format("Error updating thread count: {}", ERROR.what()));
            throw;
         }
    }
} // namespace config_manager