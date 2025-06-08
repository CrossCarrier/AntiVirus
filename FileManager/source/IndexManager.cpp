#include "../include/IndexManager.hpp"
#include "../../ERRORS_PACK/include/errors.hpp"
#include "../../HELPERS/include/Types.hpp"
#include "../../HELPERS/include/support.hpp"
#include "../../HELPERS/include/ThreadPool.hpp"
#include "../include/FileManager.hpp"
#include "../../ConfigManager/include/ConfigManager.hpp"
#include "CLI/Validators.hpp"
#include <algorithm>
#include <iostream>
#include <vector>
#include <future>

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
} // namespace

namespace {

    const auto INDEX_STORAGE_PATH = config_manager::get_index_storage_path();
    const auto LEGEND_INDEX_FILE = INDEX_STORAGE_PATH + "/Legend.json";
    const auto NUMBER_OF_THREADS = config_manager::get_number_of_threads();

} // namespace
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
        return filesToKeep;
    }
} // namespace index_manager
