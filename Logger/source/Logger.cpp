#include "../include/Logger.hpp"
#include <spdlog/fmt/fmt.h>
#include <mutex>
#include <ctime>

namespace logger {

    std::unique_ptr<Logger> Logger::instance_ = nullptr;
    std::mutex Logger::instance_mutex_;

    Logger::Logger() {
        // Create console logger with color support
        main_logger_ = spdlog::stdout_color_mt("antivirus_main");
        main_logger_->set_pattern("[%Y.%m.%d %H:%M:%S] [%^%l%$] %v");

        // Create file logger (optional, enabled via enableFileLogging)
        file_logger_ = nullptr;

        // Create debug logger
        debug_logger_ = spdlog::stdout_color_mt("antivirus_debug");
        debug_logger_->set_pattern("[%Y.%m.%d %H:%M:%S] [DEBUG] %v");
        debug_logger_->set_level(spdlog::level::off); // Disabled by default

        // Set default log level
        main_logger_->set_level(spdlog::level::info);
    }

    Logger& Logger::getInstance() {
        std::lock_guard<std::mutex> lock(instance_mutex_);
        if (instance_ == nullptr) {
            instance_ = std::unique_ptr<Logger>(new Logger());
        }
        return *instance_;
    }

    void Logger::debug(const std::string& message) {
        debug_logger_->debug(message);
    }

    void Logger::info(const std::string& message) {
        main_logger_->info(message);
        if (file_logger_) {
            file_logger_->info(message);
        }
    }

    void Logger::warn(const std::string& message) {
        main_logger_->warn(message);
        if (file_logger_) {
            file_logger_->warn(message);
        }
    }

    void Logger::error(const std::string& message) {
        main_logger_->error(message);
        if (file_logger_) {
            file_logger_->error(message);
        }
    }

    void Logger::success(const std::string& message) {
        main_logger_->info("✓ {}", message);
        if (file_logger_) {
            file_logger_->info("SUCCESS: {}", message);
        }
    }

    void Logger::logAntivirusStart() {
        info("=== ANTIVIRUS STARTED ===");
        debug("Initializing virus scanning engine...");
        debug("Loading YARA rules...");
        debug("Setting up thread pool...");
    }

    void Logger::logAntivirusShutdown() {
        info("=== ANTIVIRUS SHUTDOWN ===");
        debug("Cleaning up resources...");
    }

    void Logger::logScanStart(ScanType type, const std::string& target_path) {
        std::string type_str = scanTypeToString(type);

        if (!target_path.empty()) {
            info(std::format("{} started for: {}", type_str, target_path));
            debug(std::format("Scan target: {}", target_path));
        } else {
            info(std::format("{} started", type_str));
        }

        debug("Scan configuration loaded");
        debug("Thread pool initialized");
    }

    void Logger::logScanProgress(const std::string& current_file, int current_count, int total_count) {
        if (total_count > 0) {
            double percentage = (static_cast<double>(current_count) / total_count) * 100.0;
            debug(std::format("Scanning file {}/{} ({:.1f}%): {}", current_count, total_count, percentage, current_file));
        } else {
            debug(std::format("Scanning file: {}", current_file));
        }
    }

    void Logger::logScanResult(ScanType type, const ScanResult& result) {
        std::string type_str = scanTypeToString(type);

        if (result.success) {
            if (result.threats_found > 0) {
                warn(std::format("{}: {} FILES SCANNED, {} THREATS FOUND (Duration: {}ms)",
                     type_str, result.files_scanned, result.threats_found, result.scan_duration.count()));
            } else {
                success(std::format("{}: {} FILES SCANNED, NO THREATS FOUND (Duration: {}ms)",
                        type_str, result.files_scanned, result.scan_duration.count()));
            }
        } else {
            error(std::format("{} FAILED: {} - {} files scanned before failure",
                  type_str, result.error_message, result.files_scanned));
        }

        debug(std::format("Scan statistics: Files: {}, Threats: {}, Duration: {}ms",
              result.files_scanned, result.threats_found, result.scan_duration.count()));
    }

    void Logger::logThreatDetected(const std::string& file_path, const std::string& threat_name) {
        warn(std::format("THREAT DETECTED: {} in file: {}", threat_name, file_path));
        debug(std::format("Threat details - Name: {}, File: {}", threat_name, file_path));
    }

    void Logger::logScanError(const std::string& file_path, const std::string& err) {
        error(std::format("SCAN ERROR for file {}: {}", file_path, err));
        debug(std::format("Detailed scan error: File: {}, Error: {}", file_path, err));
    }

    void Logger::logCleaningStart() {
        info("=== CLEANING INFECTED FILES ===");
        debug("Reading infection report...");
        debug("Preparing file removal operations...");
    }

    void Logger::logCleaningResult(const CleaningResult& result) {
        if (result.success) {
            if (result.files_cleaned > 0) {
                success(std::format("CLEANING COMPLETED: {} files cleaned successfully", result.files_cleaned));
                if (result.files_failed > 0) {
                    warn(std::format("WARNING: {} files could not be cleaned", result.files_failed));
                }
            } else {
                info("CLEANING COMPLETED: No infected files found to clean");
            }
        } else {
            error(std::format("CLEANING FAILED: {}", result.error_message));
        }

        debug(std::format("Cleaning statistics: Success: {}, Failed: {}", result.files_cleaned, result.files_failed));
    }

    void Logger::logFileCleaningAttempt(const std::string& file_path) {
        debug(std::format("Attempting to clean infected file: {}", file_path));
    }

    void Logger::logFileCleaningSuccess(const std::string& file_path) {
        debug(std::format("Successfully cleaned: {}", file_path));
    }

    void Logger::logFileCleaningFailed(const std::string& file_path, const std::string& reason) {
        warn(std::format("Failed to clean {}: {}", file_path, reason));
    }

    void Logger::logIndexCreation(const std::string& index_name, const std::string& target_path) {
        info(std::format("Creating index '{}' for path: {}", index_name, target_path));
        debug(std::format("Index creation started: Name: {}, Path: {}", index_name, target_path));
    }

    void Logger::logIndexUpdate(const IndexUpdateResult& result) {
        if (result.success) {
            success(std::format("INDEX UPDATE: {} updated successfully at {}", result.index_path, result.save_location));

            auto time_t = std::chrono::system_clock::to_time_t(result.update_time);
            debug(std::format("Index update completed at: {}", std::ctime(&time_t)));
        } else {
            error(std::format("INDEX UPDATE FAILED for {}: {}", result.index_path, result.error_message));
        }
    }

    void Logger::logIndexError(const std::string& operation, const std::string& err) {
        error(std::format("INDEX {} ERROR: {}", operation, err));
    }

    void Logger::enableDebugMode(bool enable) {
        if (enable) {
            debug_logger_->set_level(spdlog::level::debug);
            info("Debug mode enabled");
        } else {
            debug_logger_->set_level(spdlog::level::off);
            info("Debug mode disabled");
        }
    }

    void Logger::enableFileLogging(const std::string& log_file_path) {
        try {
            // Create rotating file logger (5MB max, 3 files)
            file_logger_ = spdlog::rotating_logger_mt("antivirus_file", log_file_path, 1024 * 1024 * 5, 3);
            file_logger_->set_pattern("[%Y.%m.%d %H:%M:%S] [%l] %v");
            file_logger_->set_level(main_logger_->level());

            info(std::format("File logging enabled: {}", log_file_path));
        } catch (const spdlog::spdlog_ex& ex) {
            error(std::format("Failed to enable file logging: {}", ex.what()));
        }
    }

    std::string Logger::scanTypeToString(ScanType type) {
        switch (type) {
            case ScanType::FILE_SCAN: return "FILE SCAN";
            case ScanType::DIRECTORY_SCAN: return "DIRECTORY SCAN";
            case ScanType::SYSTEM_SCAN: return "SYSTEM SCAN";
            default: return "UNKNOWN SCAN";
        }
    }

    spdlog::level::level_enum Logger::toSpdlogLevel(LogLevel level) {
        switch (level) {
            case LogLevel::DEBUG: return spdlog::level::debug;
            case LogLevel::INFO: return spdlog::level::info;
            case LogLevel::WARN: return spdlog::level::warn;
            case LogLevel::ERROR: return spdlog::level::err;
            case LogLevel::SUCCESS: return spdlog::level::info;
            default: return spdlog::level::info;
        }
    }

} // namespace logger

