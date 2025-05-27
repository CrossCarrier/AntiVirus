#include "ConfigManager/include/ConfigManager.hpp"
#include "File.hpp"
#include "FileManager/include/IndexManager.hpp"
#include "HELPERS/include/support.hpp"
#include "Scanner/include/Scanner.hpp"
#include <CLI/CLI.hpp>
#include <algorithm>
#include <exception>
#include <filesystem>
#include <memory>
#include <openssl/bio.h>
#include <stdexcept>
#include <unordered_map>

namespace {
    void save_results_file(nlohmann::json &data, const std::string &path_name, SCAN_RESULTS &&results) {
        std::string is_detected = "detected";
        if (results.empty()) {
            is_detected = "detected";
        }

        try {
            data[path_name]["detection status"] = std::move(is_detected);
            data[path_name]["viruses"] = results;
            data[path_name]["number of detected viruses"] = results.size();
        } catch (std::exception &ERROR) {
            throw ERROR;
        }
    }
} // namespace
  //
namespace {
    constexpr const char *INDEX_FILE = "../antivirus/AppData/index_file.json";
    const int NUMBER_OF_THREADS = support::json_utils::read_data("../antivirus/AppData/user_settings.json")["Number of threads"];
} // namespace

int main(int argc, char *argv[]) {
    index_manager::update_metaindex(INDEX_FILE);

    CLI::App app{"Antivirus"};

    std::string file_path;
    std::string directory_path;
    std::string config_file;
    bool quick_scan_flag = false;
    bool system_scan_flag = false;
    int number_of_threads = 4;

    app.add_option("--scan_file", file_path, "File scanning");
    app.add_option("--scan_directory", directory_path, "Directory scanning");
    app.add_option("--scan_config", config_file, "Custom options for scanning located in config file");
    app.add_option("--set_threads", number_of_threads, "Set number of threads for antivirus");
    app.add_flag("--quick", quick_scan_flag, "Enables quick scans");
    app.add_flag("--system", system_scan_flag, "Scans all files on your system");

    try {
        app.parse(argc, argv);

        if (!file_path.empty()) {
            nlohmann::json data;
            save_results_file(data, file_path, scanner::scan_file(std::filesystem::path(file_path)));
            support::json_utils::write_data("output.json", data);
        }
        if (number_of_threads != 4) {
            nlohmann::json thread_info;
            thread_info["Number of threads"] = number_of_threads;
            support::json_utils::write_data("../antivirus/AppData/user_settings.json", thread_info);
        }

        if (!quick_scan_flag) {
            std::unique_ptr<std::unordered_map<std::string, SCAN_RESULTS>> scanning_results;

            if (!directory_path.empty()) {
                try {
                    if (!std::filesystem::exists(directory_path)) {
                        throw std::invalid_argument("CUSTOM ERROR NEEDED THERE!");
                    }
                    if (!std::filesystem::is_directory(directory_path)) {
                        throw std::invalid_argument("CUSTOM ERRRO NEEDED THERE!");
                    }
                    auto directory_files = support::filesystem_utils::load_from_directory(std::filesystem::path(directory_path));
                    scanning_results = std::make_unique<std::unordered_map<std::string, SCAN_RESULTS>>(
                        scanner::scanMultipleFiles(directory_files));
                } catch (std::exception &ERROR) {
                    throw ERROR;
                }
            }
            if (system_scan_flag) {
                auto systemFiles = support::filesystem_utils::load_files_from_system();
                scanning_results =
                    std::make_unique<std::unordered_map<std::string, SCAN_RESULTS>>(scanner::scanMultipleFiles(systemFiles));
            }
            if (!config_file.empty()) {
                if (!std::filesystem::exists(config_file)) {
                    throw std::invalid_argument("CUSTOM ERROR NEEDED THERE!");
                }
                try {
                    auto config_files = config_manager::fetch_config_files(std::filesystem::path(config_file));
                    scanning_results =
                        std::make_unique<std::unordered_map<std::string, SCAN_RESULTS>>(scanner::scanMultipleFiles(config_files));
                } catch (std::exception &ERROR) {
                    throw ERROR;
                }
            }

            nlohmann::json data;

            try {
                std::ranges::for_each(*scanning_results, [&data](std::pair<std::string, SCAN_RESULTS> &&file_result) -> void {
                    save_results_file(data, file_result.first, std::move(file_result.second));
                });
                support::json_utils::write_data("output.json", data);
            } catch (std::exception &ERROR) {
                throw ERROR;
            }

        } else {
            /* FETCHING MODIFIED DATA LOGIC */
        }
    } catch (const CLI::ParseError &error) {
        return app.exit(error);
    }

    return 0;
}
