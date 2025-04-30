#include "../include/errors.hpp"

BasicErrorBuild::BasicErrorBuild(const char *message) : error_message{message} {}
auto BasicErrorBuild::what() const throw() -> const char * { return this->error_message.c_str(); }