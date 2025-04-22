#pragma once

#include <filesystem>
#include <unordered_map>
#include "File.hpp"

using INDEX_LIST = std::unordered_map<std::filesystem::path, File>;
using INDEX_DATABASE_PATH = std::filesystem::path;

class IndexManager {
private:
    INDEX_DATABASE_PATH m_IndexDataBase;
    INDEX_LIST m_IndexsList;

public:
    IndexManager(const INDEX_DATABASE_PATH &IndexDataBase);
};
