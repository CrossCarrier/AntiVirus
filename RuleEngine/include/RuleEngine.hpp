#pragma once

#include <boost/filesystem/path.hpp>
#include <vector>

using SCAN_RESULTS = std::vector<std::string>;

enum class RULES_DIRS : uint8_t {
      ANTI_DEBUG_VM       = 1,
      CAPABILITIES        = 2,
      CVE_RULES           = 3,
      CRYPTO              = 4,
      EXPLOIT_KITS        = 5,
      MALICIOUS_DOCUMENTS = 6,
      MALWARE             = 7,
      PACKERS             = 8,
      WEB_SHELLS          = 9,
      EMAIL               = 10,
      MALWARE_MOBILE      = 11
};

class RuleEngine {
  private:
      static boost::filesystem::path m_ConfigFile;

  public:
      static auto scan_FILE(const boost::filesystem::path &_file_path) -> SCAN_RESULTS;
};
