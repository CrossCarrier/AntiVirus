#pragma once

#include <vector>
#include "../../FileManager/include/File.hpp"

using DirectoryFiles = std::vector<std::string>;

class Directory{
private:
    DirectoryFiles m_DirectoryFiles;
    int m_NumberOfThreats = 0;
    ThreatLevel m_AverageThreatLevel = ThreatLevel::NONE;
    std::time_t m_ModificationDate = 0;
};