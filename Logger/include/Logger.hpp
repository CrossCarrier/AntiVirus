#pragma once

/* Library we are going to use to log outputs : spdlog*/

/*
Logowania dotyczące : skanowania pliku , skanowania katalogu czy poszly dobrze czy cos sie nie udalo, status skanowania
szybkiego i calkowitego , czyli ile wirusow wykrylo , czy poszlo dobrze czy napotaklo problem , ilosc wykrytych wirusuow
w katalogu/ pliku, log zmiany czasu pomiedzy automatycznymi skanami, wyniki clearpwania pliku/katalogu/komptera, czy sie
clearowania udalo czy nie, ile wirusow oczyszczono, log z update info na temat plikow i katalogow i ten log pokazuje
rowniez gdzie zapisano te updaty i o ktorej je wykonano, log update katalogu, wyniki clearowania komputera z smieci ,
czyli sie sie powiiodlo czy nie , i ile udalo sie zwolnic miejsca z tych smieci
*/

/*
DEBUG
Logowania uruchomienia antywirusa
Poziom logowania (error, success, warn ...)
Dev debug logs -> Skanowanie pliku ... , stan zagrozenia ...

Kompatkowy raport z automatycznego skanowania i usuwania garbage

[2025.04.19] QUICK SCAN : 1200 FILES HAVE BEEN SCANNED, 8 THREATS FOUND.
[2025.04.19] GARBAGE COLLECTING : FREED 513 MB.
*/

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <memory>
#include <string>
#include <chrono>

namespace logger {

    enum class LogLevel {
        DEBUG,
        INFO,
        WARN,
        ERROR,
        SUCCESS
    };

    enum class ScanType {
        FILE_SCAN,
        DIRECTORY_SCAN,
        SYSTEM_SCAN
    };

    struct ScanResult {
        int files_scanned = 0;
        int threats_found = 0;
        std::chrono::milliseconds scan_duration{0};
        bool success = true;
        std::string error_message = "";
    };

    struct CleaningResult {
        int files_cleaned = 0;
        int files_failed = 0;
        bool success = true;
        std::string error_message = "";
    };

    struct IndexUpdateResult {
        std::string index_path = "";
        std::string save_location = "";
        std::chrono::system_clock::time_point update_time;
        bool success = true;
        std::string error_message = "";
    };

    class Logger {
    private:
        std::shared_ptr<spdlog::logger> main_logger_;
        std::shared_ptr<spdlog::logger> file_logger_;
        std::shared_ptr<spdlog::logger> debug_logger_;

        static std::unique_ptr<Logger> instance_;
        static std::mutex instance_mutex_;

        Logger();

    public:
        static Logger& getInstance();

        void debug(const std::string& message);
        void info(const std::string& message);
        void warn(const std::string& message);
        void error(const std::string& message);
        void success(const std::string& message);

        void logAntivirusStart();
        void logAntivirusShutdown();

        void logScanStart(ScanType type, const std::string& target_path = "");
        void logScanProgress(const std::string& current_file, int current_count, int total_count);
        void logScanResult(ScanType type, const ScanResult& result);
        void logThreatDetected(const std::string& file_path, const std::string& threat_name);
        void logScanError(const std::string& file_path, const std::string& error);

        void logCleaningStart();
        void logCleaningResult(const CleaningResult& result);
        void logFileCleaningAttempt(const std::string& file_path);
        void logFileCleaningSuccess(const std::string& file_path);
        void logFileCleaningFailed(const std::string& file_path, const std::string& reason);

        void logIndexCreation(const std::string& index_name, const std::string& target_path);
        void logIndexUpdate(const IndexUpdateResult& result);
        void logIndexError(const std::string& operation, const std::string& error);

        void enableDebugMode(bool enable);
        void enableFileLogging(const std::string& log_file_path);

    private:
        std::string scanTypeToString(ScanType type);
        spdlog::level::level_enum toSpdlogLevel(LogLevel level);
    };

} // namespace logger
