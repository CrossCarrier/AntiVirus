#include "../include/Scanner.hpp"
#include <boost/filesystem/fstream.hpp>
#include <fstream>
#include <iostream>

#include <thread>

#include "../../RuleEngine/include/RuleEngine.hpp"

auto Scanner::scan_file(const File &_file) -> void {

    std::thread string_match_worker(check_for_string_match, _file);
    std::thread regex_pattern_worker(check_for_regex_pattern, _file);

    SingleRuleScan_INFO str_mat = check_for_string_match(_file);
    SingleRuleScan_INFO rgx_pat = check_for_regex_pattern(_file);
    SingleRuleScan_INFO hsh_mat = check_for_hash_pattern(_file);
    SingleRuleScan_INFO bhv_pat = check_for_behaviour_pattern(_file);
}
