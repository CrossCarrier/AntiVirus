#include "../include/errors.hpp"

BasicErrorBuild::BasicErrorBuild(std::string&& message) : error_message{message} {}
auto BasicErrorBuild::what() const throw() -> const char * { return this->error_message.c_str(); }