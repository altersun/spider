#include <iostream>
#include <vector>

#include "logging.h"

namespace {
#define LOGLVL(log_level) #log_level,
    const std::vector<std::string> s_log_levels {
        LOGLVLS
    };
#undef LOGLVL

    Spider::Log::Level s_current_log_level = Spider::Log::INFO;
}

#define LOGLVL(log_level) void\
 Spider::Log_##log_level(std::string\
 to_log){Spider::Log::Log_Basic(Log::log_level,to_log);}
    LOGLVLS
#undef LOGLVL



void Spider::Log::Log_Basic(Spider::Log::Level level, std::string to_log)
{
    if (level < s_current_log_level) {
        return;
    }
    
    std::cout << s_log_levels[level] << " " << to_log << std::endl;
}


void Spider::Log::SetLevel(Spider::Log::Level level)
{
    s_current_log_level = level;
}