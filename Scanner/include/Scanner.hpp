#pragma once

class Scanner{
private:
    std::time_t m_TimeInterval = 0;
public:
    auto change_time_interval(std::time_t time_interval) -> void;

    static auto scan_file() -> void;
    static auto scan_directory() -> void;
    static auto full_scan() -> void;
    static auto quick_scan() -> void;
};