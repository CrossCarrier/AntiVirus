#pragma once
#include <yara/arena.h>
#include <yara/compiler.h>
#include <yara/libyara.h>
#include <yara/rules.h>
#include <yara/scan.h>
#include <yara/types.h>

#include <boost/filesystem/path.hpp>

class YARA_Wrapper {
private:
    static auto YARA_CALLBACK_FUNCTION(YR_SCAN_CONTEXT *context, int message, void *message_data,
                                       void *user_data) -> int;

public:
    static auto YARA_SCAN(const boost::filesystem::path &_file,
                          const boost::filesystem::path &_rules_config_file, const void *_results)
        -> void;
};
