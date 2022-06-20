#include <iostream>
#include <vector>

#include "logging.h"

namespace {
#define LOGLVL(log_level) #log_level,
    const std::vector<std::string> s_log_levels {
        LOGLVLS
    };

    Spider::Log::Level s_current_log_level = Spider::Log::INFO;
}


void Spider::Log::Log(Spider::Log::Level level, std::string to_log)
{
    if (level < s_current_log_level) {
        return;
    }
    
    std::cout << s_log_levels[level] << " " << to_log << std::endl;
}