#include "../include/Scanner.hpp"
#include "../../FileManager/include/File.hpp"
#include "../../RuleEngine/include/RuleEngine.hpp"
#include "../include/YARA_Wrapper.hpp"
#include <iostream>
#include <memory>

namespace scanner {
    /* flag --scan_file 'FILEPATH' -> Generating output.json */
    auto scan_file(const std::filesystem::path &file_path) -> SCAN_RESULTS {
        using namespace std::filesystem;
        SCAN_RESULTS results;

        for (const auto &directory_path : rule_engine::get_Rules()) {
            std::unique_ptr<directory_iterator> iterator = std::make_unique<directory_iterator>(directory_path);

            for (const auto &director_path : std::filesystem::directory_iterator(directory_path)) {
                YARA_Wrapper::YARA_SCAN(file_path, **iterator.get(), &results);
            }
        }

        return results;
    }

    /* flag --scan_dir 'DIR_PATH' -> Generating output.json */
    // auto scan_directory(const std::filesystem::path &directory_path) -> std::vector<SCAN_RESULTS> {
    //     using TREADS_CONTAINER = std::vector<std::thread>;
    //     auto files = support::filesystem_utils::load_from_directory(directory_path);

    // }

    /* flag --scan_from_config 'CONFIG_JSON_PATH' -> Generating output.json */
    /* flag --system_scan --full/--quick -> Generating output.json */
} // namespace scanner
