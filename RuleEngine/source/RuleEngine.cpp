#include "../include/RuleEngine.hpp"
#include "../../ERRORS_PACK/include/errors.hpp"
#include "../../HELPERS/include/json_manager.hpp"
#include <yara/arena.h>
#include <yara/compiler.h>
#include <yara/libyara.h>
#include <yara/rules.h>
#include <yara/scan.h>
#include <yara/types.h>

#include <boost/filesystem/directory.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <cstdarg>
#include <cstring>
#include <iostream>
#include <string>
#include <unordered_map>

RuleEngine::RuleEngine(const boost::filesystem::path &_config_file) {
    if (!boost::filesystem::exists(_config_file)) {
        std::string err_msg("Path do not exists -> ");
        err_msg += boost::filesystem::absolute(_config_file).c_str();

        throw PathNotFound(std::move(err_msg));
    }

    auto readed_data = json_manager::read_data(_config_file);
    for (const auto &rules_dir : readed_data) {
        this->m_Rules.push_back(rules_dir);
    }
}

auto RuleEngine::get_Rules() const noexcept -> const std::vector<std::string> & {
    return this->m_Rules;
}