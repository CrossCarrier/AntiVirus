#include "../include/Scanner.hpp"
#include "../../FileManager/include/File.hpp"
#include "../../HELPERS/include/FixedSizeContainer.hpp"
#include "../../HELPERS/include/json_manager.hpp"
#include "../include/YARA_Wrapper.hpp"
#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#define SUCCESS 0
#define ERROR -1

namespace {
    using DOUBLE_GROUP = FixedSizeContainer<std::string>;
    using TRIPLE_GROUP = FixedSizeContainer<std::string>;

    template <typename STRUCT_TYPE>
    auto HELPER_ScanOverRulesDir(const boost::filesystem::path &_file,
                                 STRUCT_TYPE &HARDCODED_RULES_DIRS, SCAN_RESULTS *_results)
        -> void {
        for (const auto &CONFIG : HARDCODED_RULES_DIRS) {
            boost::filesystem::directory_iterator dir_iter{CONFIG};
            while (dir_iter != boost::filesystem::directory_iterator{}) {
                YARA_Wrapper::YARA_SCAN(_file, *dir_iter, &_results);
            }
        }
    }

    inline auto HELPER_FetchRules(const RuleEngine *_rule_engine) -> std::vector<std::string> {
        return _rule_engine->get_Rules();
    }
    inline auto HELPER_FetchFiles(const Directory *_directory) -> std::vector<File> {
        return _directory->get_Files();
    }
    auto HELPER_MultiThreadScan(const std::vector<File> *_files) -> int {
        for (auto const &file : *_files) {
            std::unique_ptr<SCAN_RESULTS> resutls;
            if (Scanner::scan_file(file, resutls.get()) == ERROR) {
                std::cerr << "ERROR OCCURED - STOPING SCANNING" << std::endl;
                return ERROR;
            } else {
                file.set_ScanningResults(resutls.get());
            }
        }
        return SUCCESS;
    }
} // namespace

auto Scanner::access_to_RuleEngine() -> RuleEngine * { return supported_RuleEngine; }

auto Scanner::scan_file(const File &_file, SCAN_RESULTS *results) -> int {
    using namespace boost::filesystem;

    for (const auto &directory_path : HELPER_FetchRules(supported_RuleEngine)) {
        std::unique_ptr<directory_iterator> itterator =
            std::make_unique<directory_iterator>(directory_path);

        while (*itterator != directory_iterator{}) {
            try {
                YARA_Wrapper::YARA_SCAN(_file.get_FilePath(), **itterator, &results);
            } catch (std::exception &_) {
                std::cerr << _.what() << std::endl;
                return ERROR;
            }
        }
    }
    return SUCCESS;
}

auto Scanner::scan_directory(const Directory &_directory) -> int {
    std::vector<File> files_1;
    std::vector<File> files_2;

    for (size_t file_index = 0; file_index < _directory.get_Files().size(); file_index++) {
        auto file = _directory.get_Files().at(file_index);
        if (file_index % 2 == 0) {
            files_1.push_back(file);
        }
        files_2.push_back(file);
    }

    std::thread worker_1(HELPER_MultiThreadScan, files_1);
    std::thread worker_2(HELPER_MultiThreadScan, files_2);

    return SUCCESS;
}

auto Scanner::scan_from_config_file(const boost::filesystem::path &_config_file_path) -> int {
    auto config_data = json_manager::read_data(_config_file_path);

    /* DIRECTORIES */
    for (const auto &directory : config_data["Directories"]) {
        if (Scanner::scan_directory(Directory(boost::filesystem::path(directory))) == ERROR) {
            return ERROR;
        }
    }
    /* FILES */
    for (const auto &file : config_data["Files"]) {
        File _file(file);
        std::unique_ptr<SCAN_RESULTS> results;

        if (Scanner::scan_file(_file, results.get()) == ERROR) {
            return ERROR;
        }
        _file.set_ScanningResults(results.get());
    }

    return SUCCESS;
}