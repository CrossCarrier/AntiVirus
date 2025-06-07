#include "../include/ConfigManager.hpp"
#include "../../HELPERS/include/support.hpp"
#include "../../FileManager/include/FileManager.hpp"
#include "../../ERRORS_PACK/include/errors.hpp"
#include "../../HELPERS/include/Types.hpp"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <fstream>

namespace config_manager {
    namespace CTF = types::filesystem_types;
    namespace FS = std::filesystem;

    auto fetch_config_files(const CTF::PATH &config_file_path) -> PATHS_CONTAINER {
        CONFIG_FILES fetched_files;
        auto config_data = support::json_utils::read_data(config_file_path);

        if (config_data.contains("Directories") && config_data["Directories"].is_array()) {
            std::ranges::for_each(config_data["Directories"],
                [&fetched_files](const std::string &directory_path) -> void {
                    auto DirPathABS = FS::absolute(directory_path);

                    if (!filemanager::validate::validate_directory(DirPathABS)) {
                        throw DirectoryValidationError("Invalid directory in config: " + DirPathABS.string());
                    }

                    constexpr auto directoryIteratorOptions = ITER_OPTIONS::skip_permission_denied;
                    std::ranges::for_each(CTF::DIRECTORY_ITER_R(DirPathABS, directoryIteratorOptions),
                        [&fetched_files](const auto& entry) -> void {
                            if (!entry.is_regular_file()) return; // Skip non-regular files
                            if (!filemanager::validate::validate_file(entry.path())) {
                                throw FileValidationError("Invalid file in config scan: " + entry.path().string());
                            }
                            fetched_files.push_back(FS::absolute(entry.path())); // Use FS::absolute
                    });
            });
        } else {
            throw ConfigFileStructureError("Configuration missing 'Directories' field or it's not an array.");
        }

        if (config_data.contains("Files") && config_data["Files"].is_array()) {
            std::ranges::for_each(config_data["Files"], [&fetched_files](const std::string &file_path) -> void {
                auto FilePathABS = FS::absolute(file_path);

                if (!filemanager::validate::validate_file(FilePathABS)) {
                    throw FileValidationError("Invalid file in config: " + FilePathABS.string());
                }

                fetched_files.emplace_back(FilePathABS);
            });
        } else {
            throw ConfigFileStructureError("Configuration missing 'Files' field or it's not an array.");
        }

        return fetched_files;
    }

    auto set_index_storage_location(const CTF::PATH &new_location) -> void {
        if (!FS::exists(new_location)) {
            try {
                if (!FS::create_directories(new_location)) {
                    if (FS::exists(new_location) && !FS::is_directory(new_location)) {
                        throw DirectoryCreationError("Path '" + new_location.string() + "' exists and is not a directory.");
                    }
                    throw DirectoryCreationError("Failed to create directory '" + new_location.string() + "' (create_directories returned false).");
                }
            } catch (const FS::filesystem_error& e) {
                throw DirectoryCreationError("Failed to create directory '" + new_location.string() + "': " + e.what());
            }
        } else if (!FS::is_directory(new_location)) {
            throw DirectoryCreationError("Path '" + new_location.string() + "' exists but is not a directory.");
        }

        constexpr auto copyOptions = FS::copy_options::recursive | FS::copy_options::overwrite_existing;
        constexpr auto JSON_KEY_INDEXES = "Indexes storage path";
        const auto USER_SETTINGS_PATH = "../antivirus/AppData/user_settings.json";

        nlohmann::json userSettings;
        try {
            userSettings = support::json_utils::read_data(USER_SETTINGS_PATH);
        } catch (const std::exception& e) {
            throw ReadingSettingsError("Reading from settings file");
        }

        std::string OLD_INDEXES_DIR;

        if (userSettings.contains(JSON_KEY_INDEXES) && userSettings[JSON_KEY_INDEXES].is_string()) {
            OLD_INDEXES_DIR = userSettings[JSON_KEY_INDEXES].get<std::string>();

            if (!OLD_INDEXES_DIR.empty() && FS::exists(OLD_INDEXES_DIR)) {
                if (filemanager::validate::validate_directory(CTF::PATH(OLD_INDEXES_DIR))) {
                    if (FS::absolute(OLD_INDEXES_DIR) != FS::absolute(new_location)) {
                        try {
                            FS::copy(OLD_INDEXES_DIR, new_location, copyOptions);
                        } catch (const FS::filesystem_error& e) {
                            throw FilesystemOperationError("Failed to copy from '" + OLD_INDEXES_DIR + "' to '" + new_location.string() + "': " + e.what());
                        }
                    }
                } else {
                    throw MovingIndexStorageError("Old " + std::string(JSON_KEY_INDEXES) + " '" + OLD_INDEXES_DIR + "' is not a valid directory.");
                }
            }
        } else {
            // Logging error logic
            throw ConfigFileStructureError("Index field error");
        }

        // Updating Index Storage with new paths for each indexed directory

        if (!OLD_INDEXES_DIR.empty() && FS::exists(OLD_INDEXES_DIR) && FS::is_directory(OLD_INDEXES_DIR) &&
            FS::absolute(OLD_INDEXES_DIR) != FS::absolute(new_location)) {
            try {
                FS::remove_all(OLD_INDEXES_DIR);
                if (FS::exists(OLD_INDEXES_DIR)) {
                    throw RemovingDirectoryContentError("Failed to remove old directory '" + OLD_INDEXES_DIR + "' (still exists after remove_all).");
                }
            } catch (const FS::filesystem_error& e) {
                throw RemovingDirectoryContentError("Failed to remove old directory '" + OLD_INDEXES_DIR + "': " + e.what());
            }
        }

        // Updating paths for each indexed directory in Legend.json HelperFile
        try {
            FS::path legend_file_path = new_location / "Legend.json";

            if (FS::exists(legend_file_path)) {
                nlohmann::json legend_json_data = support::json_utils::read_data(legend_file_path);
                bool legend_was_modified = false;

                for (auto& item : legend_json_data.items()) {
                    if (item.value().is_string()) {
                        std::string current_index_file_path_str = item.value().get<std::string>();
                        FS::path current_index_file_path(current_index_file_path_str);
                        FS::path index_file_name = current_index_file_path.filename();
                        FS::path new_full_index_file_path = new_location / "IndexFiles" / index_file_name;

                        if (current_index_file_path_str != new_full_index_file_path.string()) {
                            item.value() = new_full_index_file_path.string();
                            legend_was_modified = true;
                        }
                    }
                }

                if (legend_was_modified) {
                    // Update legend file if any index storage reference was modified
                    support::json_utils::write_data(legend_file_path.string(), legend_json_data);
                }
            } else {
                std::cerr << "Warning: Legend.json not found at " << legend_file_path.string()
                          << ". Cannot update index paths within it. This may be normal if no indexes were previously defined." << std::endl;
            }
        } catch (const nlohmann::json::exception& json_ex) {
            std::cerr << "JSON error while updating Legend.json at " << (new_location / "Legend.json").string() << ": " << json_ex.what() << std::endl;
            throw ConfigFileStructureError("Failed to process Legend.json during index storage update: " + std::string(json_ex.what()));
        } catch (const FS::filesystem_error& fs_ex) {
            std::cerr << "Filesystem error concerning Legend.json at " << (new_location / "Legend.json").string() << ": " << fs_ex.what() << std::endl;
            throw FilesystemOperationError("Filesystem error during Legend.json update: " + std::string(fs_ex.what()));
        } catch (const std::exception& e) {
            std::cerr << "Unexpected error updating Legend.json: " << e.what() << std::endl;
            throw;
        }

        try {
            userSettings[JSON_KEY_INDEXES] = new_location.string();
            support::json_utils::write_data(USER_SETTINGS_PATH, userSettings);
        } catch (const std::exception& e) {
            throw UpdatingUserSettingsError("Indexes storage");
        }
    }

    auto set_output_file(const CTF::PATH &new_location) -> void {
        if (new_location.extension() != ".json") {
            throw InvalidJsonFileFormatError();
        }

        if (new_location.has_parent_path()) {
            CTF::PATH parent_dir = new_location.parent_path();
            if (!FS::exists(parent_dir)) {
                try {
                    FS::create_directories(parent_dir);
                } catch (const FS::filesystem_error& e) {
                    throw DirectoryCreationError("Failed to create parent directory for output file '" + parent_dir.string() + "': " + e.what());
                }
            } else if (!FS::is_directory(parent_dir)) {
                throw DirectoryValidationError("Parent path for output file '" + parent_dir.string() + "' is not a directory.");
            }
        }

        std::ofstream output_stream_test(new_location.string());
        if (!output_stream_test) {
            throw StreamOpeningError();
        }
        output_stream_test.close();

        constexpr auto JSON_KEY_OUTPUT_FILE = "Output file path";
        constexpr auto USER_SETTINGS_PATH = "../antivirus/AppData/user_settings.json";

        nlohmann::json userSettings;
        try {
            userSettings = support::json_utils::read_data(USER_SETTINGS_PATH);
        } catch (const std::exception& e) {
            throw ReadingSettingsError("Reading from settings file");
        }

        if (userSettings.contains(JSON_KEY_OUTPUT_FILE) && userSettings[JSON_KEY_OUTPUT_FILE].is_string()) {
            try {
                userSettings[JSON_KEY_OUTPUT_FILE] = new_location.string();
                support::json_utils::write_data(USER_SETTINGS_PATH, userSettings);
            } catch (const std::exception& e) {
                throw UpdatingUserSettingsError("Output file");
            }
        } else {
            throw ConfigFileStructureError("Output file path field cannot be detected!");
        }
    }

    auto change_number_of_threads(int newNumberOfThreads) -> void {
        if (newNumberOfThreads <= 0 ) {
            throw InvalidNumbersOfThreadsError(newNumberOfThreads);
        }

        constexpr auto JSON_KEY_THREADS_NUMBER = "Number of threads";
        constexpr auto USER_SETTINGS_PATH = "../antivirus/AppData/user_settings.json";

        JSON userSettings;
        try {
            userSettings = support::json_utils::read_data(USER_SETTINGS_PATH);
        } catch (const std::exception& e) {
            throw ReadingSettingsError("Threads field");
        }

        if (userSettings.contains(JSON_KEY_THREADS_NUMBER) && userSettings[JSON_KEY_THREADS_NUMBER].is_number_unsigned()) {
            try {
                userSettings[JSON_KEY_THREADS_NUMBER] = newNumberOfThreads;
                support::json_utils::write_data(USER_SETTINGS_PATH, userSettings);
            } catch (const std::exception& e) {
                throw UpdatingUserSettingsError("Threads");
            }
        } else {
            throw ConfigFileStructureError("Number of threads filed cannot be detected!");
        }
    }

    namespace {
        auto fetchUserSetting(std::string&& JSON_KEY, std::string&& USER_SETTINGS_PATH) -> nlohmann::basic_json<> {
            JSON userSettings;
            try {
                userSettings = support::json_utils::read_data(USER_SETTINGS_PATH);
            } catch (std::exception& ERROR) {
                throw;
            }

            if (userSettings.contains(JSON_KEY)) {
                return userSettings[JSON_KEY];
            }
            throw ReadingSettingsError("KeyError");
        }
    }

    auto get_number_of_threads() -> int {
        try {
            constexpr auto JSON_KEY_THREADS_NUMBER = "Number of threads";
            constexpr auto USER_SETTINGS_PATH = "../antivirus/AppData/user_settings.json";

            return fetchUserSetting(JSON_KEY_THREADS_NUMBER, USER_SETTINGS_PATH);
        } catch (const std::exception& ERROR) {
            throw;
        }
    }
    auto get_index_storage_path() -> std::string {
        try {
            constexpr auto JSON_KEY_INDEXES = "Indexes storage path";
            constexpr auto USER_SETTINGS_PATH = "../antivirus/AppData/user_settings.json";

            return fetchUserSetting(JSON_KEY_INDEXES, USER_SETTINGS_PATH);
        } catch (const std::exception& ERROR) {
            throw;
        }
    }
    auto get_output_file_path() -> std::string {
        try {
            constexpr auto JSON_KEY_OUTPUT_FILE = "Output file path";
            constexpr auto USER_SETTINGS_PATH = "../antivirus/AppData/user_settings.json";

            return fetchUserSetting(JSON_KEY_OUTPUT_FILE, USER_SETTINGS_PATH);
        } catch (const std::exception& ERROR) {
            throw;
        }
    }
} // namespace config_manager