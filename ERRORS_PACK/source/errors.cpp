#include "../include/errors.hpp"
#include <iostream>

BasicErrorBuild::BasicErrorBuild(std::string&& message) : error_message{message} {}
auto BasicErrorBuild::what() const throw() -> const char * { return this->error_message.c_str(); }

ReadingSettingsError::ReadingSettingsError() {
    std::cout << "Interface tips : \n";
    std::cout << "1. Check provided path of user settings file\n";
    std::cout << "2. Check structure of user settings file\n";
}
OverwritingOtherFileError::OverwritingOtherFileError() {
    std::cout << "Interface tips : \n";
    std::cout << "Choose other name for file`s path your are trying to create\n";
}
 MetaIndexDoNotExists::MetaIndexDoNotExists() {
    std::cout << "Interface tips : \n";
    std::cout << "1. Create new meta index for provided directory\n";
    std::cout << "2. Check if provided direcotory`s path is correct\n";
}


