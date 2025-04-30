#pragma once

#include <exception>
#include <string>

class BasicErrorBuild : public std::exception {
protected:
      std::string error_message;

public:
      BasicErrorBuild(const char *message);

      virtual auto what() const throw() -> const char *;
};

class InvalidContainerSize : public BasicErrorBuild {
public:
      using BasicErrorBuild::BasicErrorBuild;
};

class FullContainer : public BasicErrorBuild {
public:
      using BasicErrorBuild::BasicErrorBuild;
};

class EmptyContainer : public BasicErrorBuild {
public:
      using BasicErrorBuild::BasicErrorBuild;
};

class InvalidIndex : public BasicErrorBuild {
public:
      using BasicErrorBuild::BasicErrorBuild;
};
