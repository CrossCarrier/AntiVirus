#include "../include/Scanner.hpp"
#include "../../FileManager/include/File.hpp"
#include "../../HELPERS/include/FixedSizeContainer.hpp"
#include "../include/YARA_Wrapper.hpp"
#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
namespace {
    using DOUBLE_GROUP = FixedSizeContainer<std::string>;
    using TRIPLE_GROUP = FixedSizeContainer<std::string>;

    template <typename STRUCT_TYPE>
    /*HELPER FUNCTION FOR SCANING FILE*/
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
    auto init_ScanResults(boost::filesystem::path &_file_path) -> FINAL_RESULTS {
        File file(_file_path);
        SCAN_RESULTS file_results;
        FINAL_RESULTS final_results = {{file, file_results}};

        return final_results;
    }

} // namespace

auto Scanner::access_to_RuleEngine() -> RuleEngine * { return supported_RuleEngine; }
auto Scanner::scan_file(const boost::filesystem::path &_file_path) -> void {
    File file(_file_path);

    for (const auto &rules : Scanner::supported_RuleEngine->getRULES()) {
        YARA_Wrapper::YARA_SCAN(_file_path, rules.second, &results);
    }
}
