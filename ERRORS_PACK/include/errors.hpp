#pragma once

#include <exception>
#include <string>

class InvalidContainerSize : public std::exception {
  private:
      std::string error_message;

  public:
      InvalidContainerSize(const char *message);

      const char *what() const throw();
};