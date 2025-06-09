#include "../include/FileManager.hpp"
#include "../include/HASH-SHA256.hpp"
#include "../../ERRORS_PACK/include/errors.hpp"
#include <chrono>
#include <exception>
#include <iostream>

namespace filemanager {

    namespace validate {

        auto validate_file(const std::filesystem::path &_path) -> bool {
            if (!std::filesystem::exists(_path)) {
                return false;
            }
            if (!std::filesystem::is_regular_file(_path)) {
                return false;
            }
            return true;
        }

        auto validate_directory(const std::filesystem::path &_dir_path) -> bool {
            if (!std::filesystem::exists(_dir_path)) {
                return false;
            }
            if (!std::filesystem::is_directory(_dir_path)) {
                return false;
            }
            return true;
        }
    }

    namespace file {

        auto lastModificationTime(const std::filesystem::path &_path) -> time_t {
            if (!validate::validate_file(_path)) {
                std::cout << "PROBLEM with file " << _path.string() << std::endl;
                throw FileValidationError(_path);
            }

            const auto file_modification_time = std::filesystem::last_write_time(_path);
            const auto system_time = std::chrono::file_clock::to_sys(file_modification_time);
            const auto converted_system_time = std::chrono::time_point_cast<std::chrono::system_clock::duration>(system_time);

            return std::chrono::system_clock::to_time_t(converted_system_time);
        }

        auto size(const std::filesystem::path &_path) -> uintmax_t {
            if (!validate::validate_file(_path)) {
                std::cout << "PROBLEM with file " << _path.string() << std::endl;
                throw FileValidationError(_path);
            }

            return std::filesystem::file_size(_path);
        }

        auto hash(const std::filesystem::path &_path) -> std::string {
            if (!validate::validate_file(_path)) {
                std::cout << "PROBLEM with file " << _path.string() << std::endl;
                throw FileValidationError(_path);
            }

            std::string file_hash;
            try {
                file_hash = hash_SHA256::hash_file(_path);
            } catch (std::exception &_) {
                // Logging error logic
                throw;
            }

            return file_hash;
        }

        auto isMod(const std::filesystem::path &_path, const time_t _prev_mod, const ssize_t _prev_size,
                   const std::string &_prev_hash) -> bool {
            return (_prev_mod != lastModificationTime(_path) && _prev_size != size(_path) && _prev_hash != hash(_path));
        }
    } // namespace file
} // namespace filemanager
