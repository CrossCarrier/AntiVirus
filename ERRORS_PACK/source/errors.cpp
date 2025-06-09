#include "../include/errors.hpp"
#include <string>

BasicErrorBuild::BasicErrorBuild(std::string&& message) : error_message{message} {}
auto BasicErrorBuild::what() const throw() -> const char * { return this->error_message.c_str(); }

StreamOpeningError::StreamOpeningError():
BasicErrorBuild("Error opening file stream")
{}
auto StreamOpeningError::getTips() const noexcept -> std::string {
    return "Why it fails? Most probably wrong file`s path :P";
}

PathNotFound::PathNotFound(std::string&& filePath):
BasicErrorBuild("File path not found" + filePath) {}
auto PathNotFound::getTips() const noexcept -> std::string {
    return ("Interface tips : \n"
               "1. Check if file with such file exists`s on your system\n"
               "2. Do not include this path in your scanning\n");
}

ReadingSettingsError::ReadingSettingsError(std::string&& problemDescriptor):
BasicErrorBuild("Error reading user settings" + std::string(problemDescriptor))
{}
auto ReadingSettingsError::getTips() const noexcept -> std::string {
    return ("Interface tips : \n"
               "1. Check provided path of user settings file\n"
               "2. Check structure of user settings file\n");
}

OverwritingOtherFileError::OverwritingOtherFileError():
BasicErrorBuild("Possibility of overwriting already existing file") {}
auto OverwritingOtherFileError::getTips() const noexcept -> std::string {
     return("Interface tips : \n"
               "1. Choose other name for file`s path your are trying to create\n");
}

MetaIndexDoNotExists::MetaIndexDoNotExists():
BasicErrorBuild("Read MetaIndex does not exists") {}
auto MetaIndexDoNotExists::getTips() const noexcept -> std::string {
    return("Interface tips : \n"
              "1. Create new meta index for provided directory\n"
              "2. Check if provided directory`s path is correct\n");
}

FilesystemOperationError::FilesystemOperationError(std::string&& _error_message):
error_message{_error_message}
{}
auto FilesystemOperationError::what() const throw() -> const char * { return this->error_message.c_str(); }

DirectoryValidationError::DirectoryValidationError(const std::string& dirPath)
    : BasicErrorBuild("Directory validation error : " + dirPath) {
}
auto DirectoryValidationError::getTips() const noexcept -> std::string {
    return "Probably what ? You provided random file`s path or your directory does not exist for real";
}

FileValidationError::FileValidationError(std::string&& filePath)
    : BasicErrorBuild("File validation error.") {
}
auto FileValidationError::getTips() const noexcept -> std::string {
    return "As you can think your file is probably irregular or just does not exist";
}

ConfigFileStructureError::ConfigFileStructureError(std::string&& fieldOption)
    : BasicErrorBuild("Config file structure error : " + std::string(fieldOption)) {
}
auto ConfigFileStructureError::getTips() const noexcept -> std::string {
    return "Check for structure of your config file whether all fields are correctly named and have correct values";
}

MovingIndexStorageError::MovingIndexStorageError(const std::string& providedAction)
    : BasicErrorBuild("Error related to moving index storage.") {
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

InvalidNumberOfDaysError::InvalidNumberOfDaysError(int providedDays):
BasicErrorBuild("Provided invalid number of days : " + std::to_string(providedDays))
{}
auto InvalidNumberOfDaysError::getTips() const noexcept -> std::string {
    return "JUST PROVIDED POSITIVE NUMBER OF DAYS DUMBAS.. :P";
}

AddingTaskToCronTabError::AddingTaskToCronTabError():
BasicErrorBuild("Error creating new task for Linux crontab")
{}
auto AddingTaskToCronTabError::getTips() const noexcept -> std::string {
    return "Just check for permissions";
}




