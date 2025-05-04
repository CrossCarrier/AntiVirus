#include "../include/Scanner.hpp"
#include "../../DirectoryManager/include/Directory.hpp"
#include "../../FileManager/include/File.hpp"
#include "../../HELPERS/include/FixedSizeContainer.hpp"
#include "../../HELPERS/include/json_manager.hpp"
#include "../include/YARA_Wrapper.hpp"
#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <vector>

namespace {
    using DOUBLE_GROUP = FixedSizeContainer<std::string>;
    using TRIPLE_GROUP = FixedSizeContainer<std::string>;

    template<typename STRUCT_TYPE>
    auto HELPER_ScanOverRulesDir(const boost::filesystem::path &_file,
                                 STRUCT_TYPE &HARDCODED_RULES_DIRS, SCAN_RESULTS *_results)
        -> void {
        for (const auto &CONFIG: HARDCODED_RULES_DIRS) {
            boost::filesystem::directory_iterator dir_iter{CONFIG};
            while (dir_iter != boost::filesystem::directory_iterator{}) {
                YARA_Wrapper::YARA_SCAN(_file, *dir_iter, &_results);
            }
        }
    }

    auto init_ScanResults(boost::filesystem::path &_file_path) -> FINAL_RESULTS {
        File file(_file_path);
        SCAN_RESULTS file_results;
        FINAL_RESULTS final_results = {{file, file_results}};

        return final_results;
    }
} // namespace

auto Scanner::access_to_RuleEngine() -> RuleEngine * { return supported_RuleEngine; }

auto Scanner::scan_file(const File &_file) -> FINAL_RESULTS {
    FINAL_RESULTS scanning_results;
    auto rules_directories = supported_RuleEngine->getRULES();

    for (const auto &rules_pack: rules_directories) {
        auto abs_path_rules = boost::filesystem::absolute(rules_pack.second);
        boost::filesystem::path RULES(abs_path_rules);

        YARA_Wrapper::YARA_SCAN(_file, RULES, &scanning_results);
    }

    return scanning_results;
}

auto Scanner::scan_directory(const Directory &_directory) -> FINAL_RESULTS {
    FINAL_RESULTS scanning_results;

    return scanning_results;
}
