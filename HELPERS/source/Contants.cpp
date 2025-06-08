#include "../include/Constants.hpp"

#include <unordered_set>
#include <string>

namespace constants {

    const std::unordered_set<std::string> CriticalSystemLocalisation
    {
        "/bin", "/sbin", "/usr/bin", "/lib", "/usr/lib",
        "/usr/lib64", "/etc",
        "/usr/sbin",
        "/usr/local/bin",
        "/usr/local/lib",
        "/opt",
        "/usr/share",
        "/etc/cron.d",
        "/etc/cron.daily",
        "/etc/cron.hourly",
        "/var/lib",
    };

    const std::unordered_set<std::string> UserDirectories
    {
        "/Users",
        "/home",
        "/root",
        "/usr/local/share",
    };

    const std::unordered_set<std::string> TemporaryData
    {
        "/tmp", "/var/tmp",
        "/var/cache",
        "/var/spool",
        "/var/log",
        "/dev/shm",
    };

    const std::unordered_set<std::string> AutoStartLocations
    {
        "/etc/init.d",
        "/etc/systemd/system",
        "/etc/systemd/user",
        "/etc/xdg/autostart",
    };

    const std::unordered_set<std::string> WebDirectories
    {
        "/var/www",
        "/srv",
    };

    const std::unordered_set<std::string> ExcludeDirectories
    {
        "/proc", "/sys", "/dev", "/run",
        "/media", "/mnt",
        "/snap", "/antivirus",
    };

}