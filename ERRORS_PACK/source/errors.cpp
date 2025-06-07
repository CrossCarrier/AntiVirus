#include "../include/errors.hpp"

BasicErrorBuild::BasicErrorBuild(std::string&& message) : error_message{message} {}
auto BasicErrorBuild::what() const throw() -> const char * { return this->error_message.c_str(); }

auto ReadingSettingsError::getTips() const noexcept -> std::string {
    return ("Interface tips : \n"
               "1. Check provided path of user settings file\n"
               "2. Check structure of user settings file\n");
}
auto OverwritingOtherFileError::getTips() const noexcept -> std::string {
     return("Interface tips : \n"
               "1. Choose other name for file`s path your are trying to create\n");
}
auto MetaIndexDoNotExists::getTips() const noexcept -> std::string {
    return("Interface tips : \n"
              "1. Create new meta index for provided directory\n"
              "2. Check if provided directory`s path is correct\n");
}
auto PathNotFound::getTips() const noexcept -> std::string {
    return ("Interface tips : \n"
               "1. Check if file with such file exists`s on your system\n"
               "2. Do not include this path in your scanning\n");
}
auto StreamOpeningError::getTips() const noexcept -> std::string {
    /* No interface tips for this error*/
    return "";
}


StreamOpeningError::StreamOpeningError():
BasicErrorBuild("Error opening file stream") {}

PathNotFound::PathNotFound(std::string&& filePath):
BasicErrorBuild("File path not found" + filePath) {}

ReadingSettingsError::ReadingSettingsError(std::string&& problemDescriptor):
BasicErrorBuild("Error reading user settings" + std::string(problemDescriptor)) {
}

OverwritingOtherFileError::OverwritingOtherFileError():
BasicErrorBuild("Possibility of overwriting already existing file") {}

MetaIndexDoNotExists::MetaIndexDoNotExists():
BasicErrorBuild("Read MetaIndex does not exists") {}

#include "../include/errors.hpp"
#include <string> // Required for std::to_string

// BasicErrorBuild and some other constructors are already in your errors.cpp
// BasicErrorBuild::BasicErrorBuild(std::string&& message) : error_message{message} {}
// auto BasicErrorBuild::what() const throw() -> const char * { return this->error_message.c_str(); }

// StreamOpeningError::StreamOpeningError(): BasicErrorBuild("Error opening file stream") {}
// PathNotFound::PathNotFound(std::string&& filePath): BasicErrorBuild("File path not found" + filePath) {}
// ReadingSettingsError::ReadingSettingsError(std::string&& problemDescriptor): BasicErrorBuild("Error reading user settings") {}
// OverwritingOtherFileError::OverwritingOtherFileError(): BasicErrorBuild("Possibility of overwriting already existing file") {}
// MetaIndexDoNotExists::MetaIndexDoNotExists(): BasicErrorBuild("Read MetaIndex does not exists") {}

// Implementations for other constructors:

// Note for FilesystemOperationError:
// The class declaration in errors.hpp should be updated:
// class FilesystemOperationError final : public std::exception { // Make inheritance public
// private:
//     std::string detail_message; // Add a member to store the message
// public:
//     explicit FilesystemOperationError(std::string&& error_message);
//     [[nodiscard]] auto what() const noexcept -> const char* override; // Declare what()
// };
// The following implementation assumes these changes.
FilesystemOperationError::FilesystemOperationError(std::string&& error_message) {}

// This what() method belongs to FilesystemOperationError
// It needs to be declared in the class in errors.hpp as shown in the comment above.
/*
auto FilesystemOperationError::what() const noexcept -> const char* {
    return detail_message.c_str();
}
*/
// If you cannot modify FilesystemOperationError to add a member and what(),
// then the constructor can't effectively store the message for a standard what() mechanism.
// For now, the constructor above initializes a hypothetical 'detail_message' member.


DirectoryValidationError::DirectoryValidationError(const std::string& dirPath)
    : BasicErrorBuild("Directory validation error.") {
    // Store dirPath as a member if needed for getTips()
}
auto DirectoryValidationError::getTips() const noexcept -> std::string {
    return "";
}

FileValidationError::FileValidationError(std::string&& filePath)
    : BasicErrorBuild("File validation error.") {
    // Store filePath as a member if needed for getTips()
}
auto FileValidationError::getTips() const noexcept -> std::string {
    return "";
}

ConfigFileStructureError::ConfigFileStructureError(std::string&& fieldOption)
    : BasicErrorBuild("Config file structure error : " + std::string(fieldOption)) {
    // Store fieldOption as a member if needed for getTips()
}
auto ConfigFileStructureError::getTips() const noexcept -> std::string {
    return "";
}

MovingIndexStorageError::MovingIndexStorageError(const std::string& providedAction)
    : BasicErrorBuild("Error related to moving index storage.") {
    // Store providedAction as a member if needed for getTips()
}
auto MovingIndexStorageError::getTips() const noexcept -> std::string {
    return "";
}

DirectoryCreationError::DirectoryCreationError(std::string&& directoryPath)
    : BasicErrorBuild("Failed to create directory.") {
    // Store directoryPath as a member if needed for getTips()
}
auto DirectoryCreationError::getTips() const noexcept -> std::string {
    return "";
}

InvalidNumbersOfThreadsError::InvalidNumbersOfThreadsError(int providedNumberOfThreads)
    : BasicErrorBuild("Invalid number of threads specified: " + std::to_string(providedNumberOfThreads)) {
    // Store providedNumberOfThreads as a member if needed for getTips()
}
auto InvalidNumbersOfThreadsError::getTips() const noexcept -> std::string {
    return "";
}

RemovingDirectoryContentError::RemovingDirectoryContentError(std::string&& directoryToBeRemoved)
    : BasicErrorBuild("Error removing directory content.") {
    // Store directoryToBeRemoved as a member if needed for getTips()
}
auto RemovingDirectoryContentError::getTips() const noexcept -> std::string {
    return "";
}

InvalidJsonFileFormatError::InvalidJsonFileFormatError()
    : BasicErrorBuild("Invalid JSON file format.") {}
auto InvalidJsonFileFormatError::getTips() const noexcept -> std::string {
    return "";
}

UpdatingUserSettingsError::UpdatingUserSettingsError(std::string&& problemDescriptor)
    : BasicErrorBuild("Error updating user settings : " + std::string(problemDescriptor)) {}
auto UpdatingUserSettingsError::getTips() const noexcept -> std::string {
    return "";
}

InvalidFileIndex::InvalidFileIndex(const std::string &invalidFile):
BasicErrorBuild("Error reading data about file from index")
{}
auto InvalidFileIndex::getTips() const noexcept -> std::string {
    return "";
}

