#pragma once

#include <filesystem>
#include <vector>

#include "../../FileManager/include/File.hpp"

class Directory {
  private:
      boost::filesystem::path m_DirectoryPath;
      int m_NumberOfThreats            = 0;
      ThreatLevel m_AverageThreatLevel = ThreatLevel::NONE;
      std::time_t m_ModificationDate   = 0;

  public:
      Directory(std::filesystem::path working_directory_path);

      auto get_NumberOfThreats() const -> int;
      auto get_AverageThreatLevel() const -> ThreatLevel;
      auto isDangerous() const -> bool;
};
