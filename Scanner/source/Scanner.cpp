#include "../include/Scanner.hpp"
#include "../../FileManager/include/File.hpp"
#include "../../RuleEngine/include/RuleEngine.hpp"
#include "../include/YARA_Wrapper.hpp"
#include <algorithm>
#include <iostream>
#include <unordered_map>

namespace scanner {
    /* flag --scan_file 'FILEPATH' -> Generating output.json */
    auto scan_file(const std::filesystem::path &file_path) -> SCAN_RESULTS {
        using namespace std::filesystem;
        SCAN_RESULTS results;

        for (const auto &directory_path : rule_engine::get_Rules()) {

            if (!exists(directory_path)) {
                std::cerr << "Directory path " << directory_path.c_str() << "do not exists!" << std::endl;
                continue;
            }

            std::vector<path> loaded_rules;
            for (const auto &rule_file : directory_iterator(directory_path)) {
                if (rule_file.is_regular_file()) {
                    loaded_rules.push_back(rule_file);
                }
            }

            std::ranges::for_each(loaded_rules, [&](const path &rule_path) -> void {
                if (exists(rule_path) && rule_path.extension() == ".yar") {
                    try {
                        YARA_Wrapper::YARA_SCAN(file_path, rule_path, &results);
                    } catch (...) {
                        std::cerr << "Error occured while scanning by rule : " << rule_path.c_str() << std::endl;
                    }
                }
            });
        }

        return results;
    }

    auto scan_directory(const std::filesystem::path &directory_path) -> std::unordered_map<std::string, SCAN_RESULTS> {
        std::unordered_map<std::string, SCAN_RESULTS> result{};

        return result;
    }
    auto scan_directory(const std::vector<std::filesystem::path> &files) -> std::unordered_map<std::string, SCAN_RESULTS> {

        return {};
    }

    // auto scan_directory(const std::filesystem::path &directory_path)
    /* flag --scan_dir 'DIR_PATH' -> Generating output.json */
    // auto scan_directory(const std::filesystem::path &directory_path) -> std::vector<SCAN_RESULTS> {
    //     using TREADS_CONTAINER = std::vector<std::thread>;
    //     auto files = support::filesystem_utils::load_from_directory(directory_path);

    // }

    /* flag --scan_from_config 'CONFIG_JSON_PATH' -> Generating output.json */
    /* flag --system_scan --full/--quick -> Generating output.json */
} // namespace scanner
