#pragma once

#include <exception>
#include <string>

class BasicErrorBuild : public std::exception {
protected:
    std::string error_message;

public:
    explicit BasicErrorBuild(std::string &&message);

    [[nodiscard]] auto what() const noexcept -> const char * override;
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

class PathNotFound : public BasicErrorBuild {
public:
    using BasicErrorBuild::BasicErrorBuild;
};

class StreamOpeningError : public BasicErrorBuild {
public:
    using BasicErrorBuild::BasicErrorBuild;
};