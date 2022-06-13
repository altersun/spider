#include <cmath>
#include <sys/time.h>
#include <sys/timerfd.h>
#include <unordered_map>
#include <unistd.h>

#include "timer.h"


// Aonymous namespace
namespace {
    std::unordered_map<int, Spider::TimerHandlePtr> s_timer_handles;
}

static Spider::TimerHandlePtr AddTimer(Spider::Seconds sec, Spider::Callback cb, bool repeat);


Spider::TimerHandle::~TimerHandle()
{
    ::close(m_fd);
}


int Spider::TimerHandle::GetFD() 
{
    return m_fd;
}


Spider::TimerHandle::TimerHandle(Spider::Seconds seconds, bool repeat)
 : m_time(seconds)
 , m_repeat(repeat) 
{
    // TODO: Make this #define to use CLOCK_BOOTTIME if ALARM not supported
    // TODO: See if the TFD_NONBLOCK flag is needed for the 2nd argument
    // https://man7.org/linux/man-pages/man2/timerfd_create.2.html
    int m_fd = timerfd_create(CLOCK_BOOTTIME_ALARM, 0);
    if (m_fd == -1) {
        throw Spider::SpiderException("Could not obtain timer!");
    }

    m_spec = {0};
    ::timespec ts = Spider::ConvertSecondsToTimespec(m_time);
    if (m_repeat) {
        m_spec.it_interval = ts;
    } else {
        m_spec.it_value = ts;
    }

    timerfd_settime(m_fd, 0, &m_spec, NULL);
}


float Spider::TimerHandle::GetAssignedTime()
{
    return m_time;
}


Spider::Seconds Spider::TimerHandle::GetTimeRemaining()
{
    itimerspec interval = {0};
    int ret = timerfd_gettime(m_fd, &interval);
    if (ret != 0) {
        // TODO: Logging of what went wrong
        return -1.0;
    }
    return static_cast<Spider::Seconds>(interval.it_value.tv_sec + interval.it_value.tv_nsec*100000);
}


bool Spider::TimerHandle::IsRepeating()
{
    return m_repeat;
}


void Spider::TimerHandle::Stop()
{
    Spider::RemoveFD(GetFD());
    s_timer_handles.erase(GetFD());
}


Spider::TimerHandlePtr AddTimer(Spider::Seconds sec, Spider::Callback cb, bool repeat)
{
    Spider::TimerHandlePtr timer_p = nullptr;
    try {
        Spider::TimerHandlePtr timer_p = std::make_shared<Spider::TimerHandle>(sec, repeat);
    } catch(Spider::SpiderException) {
        return nullptr;
    }

    try {
        // TODO: Do something with the ID
        Spider::AddFD(timer_p->GetFD(), cb);
    } catch(Spider::SpiderException) {
        timer_p.reset();
        return nullptr;
    }

    s_timer_handles[timer_p->GetFD()] = timer_p;


    return timer_p;
}


Spider::TimerHandlePtr Spider::CallLater(Spider::Seconds delay, Spider::Callback cb)
{
    // TODO: Wrap callback in something that closes timer...by calling stop??
    return AddTimer(delay, cb, false);
}


Spider::TimerHandlePtr Spider::CallEvery(Spider::Seconds increment, Spider::Callback cb)
{
    return AddTimer(increment, cb, true);
}