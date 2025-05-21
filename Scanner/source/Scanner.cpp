#include "../include/Scanner.hpp"
#include "../../FileManager/include/File.hpp"
#include "../../HELPERS/include/support.hpp"
#include "../../RuleEngine/include/RuleEngine.hpp"
#include "../include/YARA_Wrapper.hpp"
#include <algorithm>
#include <expected>
#include <memory>
#include <thread>
#include <vector>

namespace {
    using PATHS_CONTAINER = std::vector<std::filesystem::path>;

    auto scan_multiple_files(const PATHS_CONTAINER &group_of_file_paths) -> void {
        std::for_each(group_of_file_paths.begin(), group_of_file_paths.end(),
                      [](const std::filesystem::path &single_file_path) -> void {
                          std::unique_ptr<SCAN_RESULTS> results;
                          scanner::scan_file(single_file_path, results.get());
                      });
    }
} // namespace

namespace scanner {
    /* flag --scan_file 'FILEPATH' -> Generating output.json */
    auto scan_file(const std::filesystem::path &file_path) -> SCAN_RESULTS {
        using namespace std::filesystem;
        SCAN_RESULTS results;

        for (const auto &directory_path : rule_engine::get_Rules()) {
            std::unique_ptr<directory_iterator> iterator = std::make_unique<directory_iterator>(directory_path);

            while (*iterator != directory_iterator{}) {
                YARA_Wrapper::YARA_SCAN(file_path, **iterator.get(), &results);
            }
        }

        return results;
    }

    /* flag --scan_dir 'DIR_PATH' -> Generating output.json */
    auto scan_directory(const std::filesystem::path &directory_path) -> std::vector<SCAN_RESULTS> {
        using TREADS_CONTAINER = std::vector<std::thread>;
        auto files = support::filesystem_utils::load_from_directory(directory_path);

        
    }

    /* flag --scan_from_config 'CONFIG_JSON_PATH' -> Generating output.json */
    /* flag --system_scan --full/--quick -> Generating output.json */
} // namespace scanner
