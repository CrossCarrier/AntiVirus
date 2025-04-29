#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <iostream>

#include "RuleEngine/include/RuleEngine.hpp"

SCAN_RESULTS g_Results;

int main() {
      boost::filesystem::path malware("/home/tralaleilotralala/code/antivirus/rules-master/malware/"
                                      "MALW_Miscelanea_Linux.yar");
      boost::filesystem::path file("/home/tralaleilotralala/code/antivirus/virus.txt");

      boost::filesystem::path file1(absolute(file));

      RuleEngine::runYaraScan(file1, malware, &g_Results);

      if (g_Results.empty()) {
            std::cout << "Rule no matched" << std::endl;
      } else {
            for (const auto &result : g_Results) {
                  std::cout << "Matched rule : " << result << std::endl;
            }
      }
      return 0;
}
