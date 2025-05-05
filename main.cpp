#include "FileManager/include/File.hpp"
#include "HELPERS/include/json_manager.hpp"
#include "RuleEngine/include/RuleEngine.hpp"
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <iostream>

int main() {
    RuleEngine YARA_RuleEngine("../antivirus/YARA_config.json");
    YARA_RuleEngine.show();

    
    return 0;
}
