#include "../include/CronManager.hpp"
#include "../../Logger/include/Logger.hpp"
#include "../../ERRORS_PACK/include/errors.hpp"
#include <filesystem>

namespace
{
    namespace helper_function
    {
        auto validateNumberOfDays(const int providedDays) -> bool
        {
            if (providedDays <= 0) return false;
            return true;
        }
    }
}

namespace cron_manager
{
    auto setupCronJob(const int days) -> void
    {
        using namespace helper_function;
        namespace FS = std::filesystem;

        auto &actionLogger = logger::Logger::getInstance();
        actionLogger.info("Configuring task in Linux Cron harmonogram");

        if (!validateNumberOfDays(days))
        {
            actionLogger.error("Number of days for scanning harmonogram must be positive");
            throw InvalidNumberOfDaysError(days);
        }

        // Obtaining full canonical path to executable file
        std::string fullCanonicalPath;
        try
        {
            fullCanonicalPath = FS::canonical("/proc/self/exe");
        }
        catch (const FS::filesystem_error& FILESYSTEM_ERROR)
        {
            actionLogger.error("Error appeared while obtaining full path of program : " + std::string(FILESYSTEM_ERROR.what()));
            actionLogger.warn("Asure Antivirus is being ran in Linux environment");
            throw;
        }
        actionLogger.debug("Full program path : " + fullCanonicalPath);

        // Building Cron-type task schedule
        // For now only available in days format
        const std::string cron_job_line = "0 0 */" + std::to_string(days) + " * * " + fullCanonicalPath + " --system --quick";
        actionLogger.info("New Cron task : " + cron_job_line);

        std::string command_to_execute = "(crontab -l 2>/dev/null; echo \"" + cron_job_line + "\") | crontab -";
        actionLogger.debug("Executed command : " + command_to_execute);

        int result = system(command_to_execute.c_str());

        if (result == 0)
        {
            actionLogger.success("Successfully added new task to crontab");
            actionLogger.info("To verify use : crontab -l in your terminal");
        }
        else
        {
            actionLogger.error("Error! Unable to add provided task to crontab");
            throw AddingTaskToCronTabError();
        }
    }

}