#pragma once

#include <exception>
#include <stdexcept>
#include <string>

class BasicErrorBuild : public std::exception {
protected:
    std::string error_message;

public:
    BasicErrorBuild() = default;
    explicit BasicErrorBuild(std::string &&message);

    [[nodiscard]] auto what() const noexcept -> const char * override;
    [[nodiscard]] virtual auto getTips() const noexcept -> std::string = 0;
};

class FilesystemOperationError final : std::exception {
public:
    explicit FilesystemOperationError(std::string&& error_message);
};

class StreamOpeningError final : public BasicErrorBuild {
public:
    StreamOpeningError();
    [[nodiscard]]auto getTips() const noexcept -> std::string override;
};
class PathNotFound final : public BasicErrorBuild {
public:
    [[nodiscard]]auto getTips() const noexcept -> std::string override;
    explicit PathNotFound(std::string&& filePath);
};
class ReadingSettingsError final : public BasicErrorBuild {
public:
    [[nodiscard]]auto getTips() const noexcept -> std::string override;
    explicit ReadingSettingsError(std::string&& problemDescriptor);
};
class OverwritingOtherFileError final : public BasicErrorBuild {
public:
    [[nodiscard]]auto getTips() const noexcept -> std::string override;
    OverwritingOtherFileError();
};
class MetaIndexDoNotExists final : public BasicErrorBuild {
public:
    [[nodiscard]]auto getTips() const noexcept -> std::string override;
    MetaIndexDoNotExists();
};
class DirectoryValidationError final : public BasicErrorBuild {
public:
    [[nodiscard]]auto getTips() const noexcept -> std::string override;
    explicit DirectoryValidationError(const std::string& dirPath);
};
class FileValidationError final : public BasicErrorBuild {
public:
    [[nodiscard]] auto getTips() const noexcept -> std::string override;
    explicit FileValidationError(std::string&& filePath);
};
class ConfigFileStructureError final : public BasicErrorBuild {
public:
    [[nodiscard]] auto getTips() const noexcept -> std::string override;
    explicit ConfigFileStructureError(std::string&& fieldOption);
};
class MovingIndexStorageError final : public BasicErrorBuild {
public:
    [[nodiscard]] auto getTips() const noexcept -> std::string override;
    explicit MovingIndexStorageError(const std::string& providedAction);
};
class DirectoryCreationError final : public BasicErrorBuild {
public:
    [[nodiscard]] auto getTips() const noexcept -> std::string override;
    explicit DirectoryCreationError(std::string&& directoryPath);
};
class InvalidNumbersOfThreadsError final : public BasicErrorBuild {
public:
    [[nodiscard]] auto getTips() const noexcept -> std::string override;
    explicit InvalidNumbersOfThreadsError(int providedNumberOfThreads);
};
class RemovingDirectoryContentError final : public BasicErrorBuild {
public:
    [[nodiscard]] auto getTips() const noexcept -> std::string override;
    explicit RemovingDirectoryContentError(std::string&& directoryToBeRemoved);
};
class InvalidJsonFileFormatError final : public BasicErrorBuild {
public:
    [[nodiscard]] auto getTips() const noexcept -> std::string override;
    InvalidJsonFileFormatError();
};
class UpdatingUserSettingsError final : public BasicErrorBuild {
public:
    [[nodiscard]] auto getTips() const noexcept -> std::string override;
    explicit UpdatingUserSettingsError(std::string&& problemDescriptor);
};
class InvalidFileIndex final : public BasicErrorBuild {
public:
    [[nodiscard]] auto getTips() const noexcept -> std::string override;
    explicit InvalidFileIndex(const std::string& invalidFile);
};