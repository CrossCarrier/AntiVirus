#include "../include/errors.hpp"

InvalidContainerSize::InvalidContainerSize(const char *_message) : error_message{_message} {}
const char *InvalidContainerSize::what() const throw() { return this->error_message.c_str(); }