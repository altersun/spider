#include <cmath>
#include <sys/time.h>
#include <sys/timerfd.h>

#include <file_handler.h>

Spider::FileDescriptor::FileDescriptor(int fid, Callback cb) :
    m_fid(fid),
    m_callback(cb)
{}



Spider::Timer::Timer(float mSec, Callback cb)
{
    time_t seconds = static_cast<time_t>(std::floor(mSec/1000));
    
    struct timespec time =  {
        seconds,
        (static_cast<long>(mSec) - (seconds*1000))
    };

    const itimerspec interval = {
        0,
        time
    };
    
    // TODO: Make this #define to use CLOCK_BOOTTIME if ALARM not supported
    // TODO: See if the TFD_NONBLOCK flag is needed for the 2nd argument
    // https://man7.org/linux/man-pages/man2/timerfd_create.2.html
    int timer = timerfd_create(CLOCK_BOOTTIME_ALARM, 0);
    if (timer == -1) {
        throw Spider::SpiderException("Could not obtain timer!");
    }

    timerfd_settime(timer, 0, &interval, NULL);
}



