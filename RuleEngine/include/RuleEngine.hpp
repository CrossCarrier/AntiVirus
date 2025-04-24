#pragma once

#include <boost/filesystem/path.hpp>
#include <filesystem>
#include "../../FileManager/include/File.hpp"

class RuleEngine {
private:
    static std::filesystem::path m_ConfigFile;

public:
    static auto set_RuleConfigFile(const boost::filesystem::path &_ConfigFile) -> void;
};
