#include "../include/IndexManager.hpp"
#include "../../ERRORS_PACK/include/errors.hpp"
#include "../../HELPERS/include/Types.hpp"
#include "../../HELPERS/include/support.hpp"
#include "../../HELPERS/include/ThreadPool.hpp"
#include "../include/FileManager.hpp"
#include "../../ConfigManager/include/ConfigManager.hpp"
#include <algorithm>
#include <iostream>
#include <vector>
#include <future>

namespace {

    const auto INDEX_STORAGE_PATH = config_manager::get_index_storage_path();
    const auto LEGEND_INDEX_FILE = INDEX_STORAGE_PATH + "/Legend.json";
    const auto NUMBER_OF_THREADS = config_manager::get_number_of_threads();

} // namespace

namespace {
    template <typename T>
    auto add_property(JSON &json, const std::string &key, const T& value) -> void {
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
        std::ranges::for_each(files, [&](const PATH &val) -> void {
            try {
                json[val.c_str()] = create_file_entry(val);
            } catch (...) {
            }
        });
    }

    auto updateFileIndex(const PATH& filePath) -> void {
        if (!filemanager::validate::validate_file(filePath)) {
            throw FileValidationError(filePath.string());
        }

        JSON LegendData = support::json_utils::read_data(LEGEND_INDEX_FILE);
        bool already_exists_in_database = false;

        std::ranges::for_each(LegendData, [&](const std::string& legendValue) -> void {
            try {
                JSON ValueData = support::json_utils::read_data(legendValue);

                if (ValueData.contains(filePath.string())) {
                    auto& KEY = ValueData[filePath.string()];

                    KEY["Hash"] = filemanager::file::hash(filePath);
                    KEY["Modification time"] = filemanager::file::hash(filePath);
                    KEY["Size"] = filemanager::file::hash(filePath);

                    already_exists_in_database = true;
                }
            } catch (std::exception& ERROR) {
                // Logging logic
                throw;
            }
        });

        if (!already_exists_in_database) {
            if (filePath.has_parent_path()) {
                const auto ParentPathOfNewFile = filePath.parent_path();
                if (LegendData.contains(ParentPathOfNewFile.string()) && LegendData[ParentPathOfNewFile.string()].is_string()) {
                    try {
                        auto ParentDirectoryIndex = std::filesystem::path(LegendData[ParentPathOfNewFile.string()].get<std::string>());
                        JSON PARENT_DIRECTORY = std::move(support::json_utils::read_data(ParentDirectoryIndex));
                        PARENT_DIRECTORY[filePath.string()] = {
                            {"Hash",std::move(filemanager::file::hash(filePath))},
                            {"Modification time", filemanager::file::lastModificationTime(filePath)},
                            {"Size", filemanager::file::size(filePath)}
                        };
                        support::json_utils::write_data(ParentDirectoryIndex, PARENT_DIRECTORY);
                    } catch (std::exception& ERROR) {
                        // Logging error logic
                        throw;
                    }
                }
            }
            else if (LegendData.contains("SYSTEM") && LegendData["SYSTEM"].is_string()) {
                const auto SYS_IDX_PATH = std::filesystem::path(LegendData["SYSTEM"].get<std::string>());
                try {
                    JSON SYSTEM_INDEXES = std::move(support::json_utils::read_data(SYS_IDX_PATH));
                    SYSTEM_INDEXES[filePath.string()] = {
                        {"Hash",std::move(filemanager::file::hash(filePath))},
                        {"Modification time", filemanager::file::lastModificationTime(filePath)},
                        {"Size", filemanager::file::size(filePath)}
                    };

                    support::json_utils::write_data(SYS_IDX_PATH, SYSTEM_INDEXES);
                } catch (std::exception& ERROR) {
                    // Logging error logic
                    throw;
                }
            }
            else {
                std::cerr << "Consider creating new database for directory : " << filePath.parent_path() << std::endl;
                std::cerr << "Update existing or create SYSTEM indexes storage to fetch data about new files" << std::endl;
            }
        }
    } // namespace
}
    namespace index_manager {

        auto createMetaIndex(const std::string &inputPath, const std::string &nameOutputPath) -> void {

            if (std::filesystem::exists(nameOutputPath)) {
                std::cerr << "Metaindex`s path already in use\n";
                throw OverwritingOtherFileError();
            }

            JSON filesIndexLegend = support::json_utils::read_data(LEGEND_INDEX_FILE);
            auto ABSDirPath = std::filesystem::absolute(std::filesystem::path(inputPath));

            if (filesIndexLegend.contains(ABSDirPath)) {
                std::cerr << "MetaIndex for this directory already exists" << std::endl;
                std::cout << "Data about files from this directory are stored in : "
                          << support::json_utils::read_data(LEGEND_INDEX_FILE)[inputPath].get<std::string>() << "\n";
                return;
            }

            std::string indexFileName = std::move(const_cast<std::string &>(nameOutputPath)) + ".json";

            indexFileName = INDEX_STORAGE_PATH + "/IndexFiles/" + indexFileName;
            JSON indexJsonData;

            if (inputPath != "system") {
                update_json_with_files(indexJsonData, support::filesystem_utils::load_from_directory(ABSDirPath));
                filesIndexLegend[ABSDirPath] = indexFileName;
            }
            else {
                update_json_with_files(indexJsonData, support::filesystem_utils::load_files_from_system());
                filesIndexLegend["SYSTEM"] = indexFileName;
            }

            try {
                support::json_utils::write_data(LEGEND_INDEX_FILE, filesIndexLegend);
                support::json_utils::write_data(indexFileName, indexJsonData);
            } catch (std::exception& ERROR) {
                // Logging error logic
                throw;
            }
        }

        auto updateMetaIndex(const std::string &inputPath) -> void {

            JSON filesIndexLegend = support::json_utils::read_data(LEGEND_INDEX_FILE);

            if (!filesIndexLegend.contains(inputPath)) {
                std::cerr << "Wanted MetaIndex do not exists\n";
                throw MetaIndexDoNotExists();
            }

            auto foundIndexPath = std::move(filesIndexLegend[inputPath]);
            JSON updatedIndex;

            if (inputPath != "SYSTEM") {
                update_json_with_files(updatedIndex, support::filesystem_utils::load_from_directory(inputPath));
            } else {
                update_json_with_files(updatedIndex, support::filesystem_utils::load_files_from_system());
            }

            support::json_utils::write_data(foundIndexPath, updatedIndex);
        }

        auto filterModified(PATHS_CONTAINER &files_input_ref) -> PATHS_CONTAINER {
            if (!filemanager::validate::validate_directory(INDEX_STORAGE_PATH)) {
                std::cerr << "Error: Index storage path is not a valid directory: " << INDEX_STORAGE_PATH << std::endl;
                throw DirectoryValidationError(INDEX_STORAGE_PATH);
            }

            if (files_input_ref.empty()) {
                return {};
            }

            JSON SystemIndexData;
            bool systemIndexLoadedSuccessfully = false;

            try {
                if (std::filesystem::exists(LEGEND_INDEX_FILE)) {
                    JSON indexJsonLegend = support::json_utils::read_data(LEGEND_INDEX_FILE);
                    if (indexJsonLegend.contains("SYSTEM") && indexJsonLegend["SYSTEM"].is_string()) {
                        std::string systemIndexPathStr = indexJsonLegend["SYSTEM"].get<std::string>();
                        if (!systemIndexPathStr.empty() && std::filesystem::exists(systemIndexPathStr)) {
                            SystemIndexData = support::json_utils::read_data(std::filesystem::path(systemIndexPathStr));
                            if (!SystemIndexData.empty()) {
                                systemIndexLoadedSuccessfully = true;
                            } else {
                                std::cerr << "Warning: SYSTEM index file at " << systemIndexPathStr << " is empty. All files will be treated as new/modified." << std::endl;
                            }
                        } else {
                            std::cerr << "Warning: SYSTEM index path '" << systemIndexPathStr << "' is invalid or file does not exist. Cannot filter." << std::endl;
                        }
                    } else {
                        std::cerr << "Warning: 'SYSTEM' entry not found or not a string in Legend.json. Cannot filter using SYSTEM index." << std::endl;
                    }
                } else {
                    std::cerr << "Warning: Index Legend file does not exist at " << LEGEND_INDEX_FILE
                              << ". Cannot filter using SYSTEM index." << std::endl;
                }
            } catch (const std::exception& e) {
                std::cerr << "Error during SYSTEM index data loading: " << e.what()
                          << ". All files will be treated as new/modified." << std::endl;
            }

            if (!systemIndexLoadedSuccessfully) {
                std::cerr << "SYSTEM index could not be loaded or is empty. All input files will be kept." << std::endl;
                return files_input_ref;
            }

            ThreadPool threadPool(NUMBER_OF_THREADS);
            std::vector<std::future<bool>> taskFutures;
            taskFutures.reserve(files_input_ref.size());

            std::mutex system_index_data_mutex;

            // Task returns:
            // true  -> REMOVE file (confirmed NOT modified)
            // false -> KEEP file (IS modified, new, or error occurred during check)
            auto check_file_task =
                [&SystemIndexData, &system_index_data_mutex](const PATH &filePath) -> bool {
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
                                    return false;
                                } else {
                                    return true;
                                }
                            } else {
                                std::cerr << "Warning: Malformed index entry for " << absPATH_str << " (missing fields). Keeping file." << std::endl;
                                return false;
                            }
                        } else {
                            return false;
                        }
                    } catch (const nlohmann::json::exception &e) {
                        std::cerr << "Warning: JSON exception for " << filePath.string() << " against SYSTEM index: " << e.what() << ". Keeping file." << std::endl;
                        return false;
                    } catch (const std::filesystem::filesystem_error& efs) {
                        std::cerr << "Warning: Filesystem error for " << filePath.string() << " (e.g. std::filesystem::absolute): " << efs.what() << ". Keeping file." << std::endl;
                        return false;
                    } catch (const std::exception &e) {
                        std::cerr << "Warning: Standard exception for " << filePath.string() << " against SYSTEM index: " << e.what() << ". Keeping file." << std::endl;
                        return false;
                    } catch (...) {
                        std::cerr << "Warning: Unknown exception for " << filePath.string() << " against SYSTEM index. Keeping file." << std::endl;
                        return false;
                    }
            };

            for (const auto& filePath : files_input_ref) {
                taskFutures.emplace_back(threadPool.addTask(check_file_task, filePath));
            }

            PATHS_CONTAINER filesToKeep;
            filesToKeep.reserve(files_input_ref.size());

            for (size_t i = 0; i < taskFutures.size(); ++i) {
                bool should_remove = true;
                try {
                    should_remove = taskFutures.at(i).get();
                } catch (const std::exception& e) {
                    std::cerr << "Exception from task future for file "
                              << (i < files_input_ref.size() ? files_input_ref.at(i).string() : "unknown_file_due_to_index_mismatch")
                              << ": " << e.what() << ". Keeping file." << std::endl;
                    should_remove = false;
                }

                if (!should_remove) {
                    if (i < files_input_ref.size()) {
                        filesToKeep.push_back(files_input_ref.at(i));
                    } else {
                        std::cerr << "Error: Index mismatch between task futures and input files. Cannot keep file for future at index " << i << std::endl;
                    }
                }
            }

            std::ranges::for_each(filesToKeep, [&](const PATH& filePath) -> void {
                updateFileIndex(filePath);
            });

            return filesToKeep;
        }

    auto updateAfterRemoval(const PATH& filePath) -> void {
            if (!filemanager::validate::validate_file(filePath)) {
                throw FileValidationError(filePath.string());
            }

            JSON LegendData;
            try {
                LegendData = support::json_utils::read_data(LEGEND_INDEX_FILE);
            } catch (std::exception& ERROR) {
                std::cerr << "Warning : Error reading Legend data from Index Database" << std::endl;
                // Logging error logic
                throw;
            }

            std::ranges::for_each(LegendData, [&](const std::string& LegendValue) -> void {
                JSON SingleIndexStorage;

                try {
                    SingleIndexStorage = support::json_utils::read_data(LegendValue);
                } catch (std::exception& ERROR) {
                    std::cerr << "Warning : Error reading data from provided Index Storage Path : " << LegendValue << std::endl;
                    throw;
                }

                if (SingleIndexStorage.contains(filePath.string())) {
                    SingleIndexStorage.erase(filePath.string());
                    try {
                        support::json_utils::write_data(std::filesystem::path(LegendValue), SingleIndexStorage);
                    } catch (std::exception& ERROR) {
                        std::cerr << "Warning : Error writing Index data after file removal" << std::endl;
                        std::cerr << "Problem with : " << LegendValue << " index storage and " << filePath.string() << std::endl;
                        // Logging error
                        throw;
                    }
                }
            });
        }
}
