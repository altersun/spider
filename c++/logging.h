#include <string>

namespace Spider::Log {


// X-Macro
#define LOGLVLS \
    LOGLVL(DEBUG) \
    LOGLVL(INFO) \
    LOGLVL(WARNING) \
    LOGLVL(ERROR) \
    LOGLVL(FATAL)

#define LOGLVL(log_level) log_level, 
enum Level {
    LOGLVLS
};
#undef LOGLVL


void Log(Level level, std::string to_log);

void SetLevel(Level level);


} // end namespace Spider::Log


//namespace Spider {
//#define LOGFUNC(log_level,to_log) using\
//   Log::log_level=Log::Log(log_level,to_log)
//#define LOGLVL(log_level) LOGFUNC()
//}



