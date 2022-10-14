#include <cmath>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/timerfd.h>
#include <unordered_map>
#include <unistd.h>

#include "logging.hpp"
#include "timer.hpp"


// Aonymous namespace
namespace {
    std::unordered_map<int, Spider::TimerHandlePtr> s_timer_handles;
}

static Spider::TimerHandlePtr AddTimer(Spider::Seconds sec, Spider::Callback cb, bool repeat);
static uint64_t ReadTimerFd(Spider::TimerHandlePtr t_ptr);
static Spider::Return TimerCallback(Spider::TimerHandlePtr timer_p, Spider::Callback cb);


Spider::TimerHandle::~TimerHandle()
{
    ::close(m_fd);
}


int Spider::TimerHandle::GetFD() 
{
    Spider::Log_DEBUG("Return timerfd with fd value "+std::to_string(m_fd));
    return m_fd;
}


Spider::TimerHandle::TimerHandle(Spider::Seconds seconds, bool repeat)
 : m_time(seconds)
 , m_repeat(repeat) 
{
    // TODO: Make this #define to use CLOCK_BOOTTIME if ALARM not supported
    // TODO: See if the TFD_NONBLOCK flag is needed for the 2nd argument
    // https://man7.org/linux/man-pages/man2/timerfd_create.2.html
    m_fd = timerfd_create(CLOCK_BOOTTIME, 0);
    if (m_fd < 0) {
        throw Spider::SpiderException("Could not obtain timer, exit code "+std::string(::strerror(errno)));
    }

    // Make fd non-blocking so we don't stall on it in the case of a mis-timed read
    int ret = ::fcntl(m_fd, F_SETFL, O_NONBLOCK);
    if (ret < 0) {
        throw Spider::SpiderException("Could make timer read non-blocking, exit code "+std::string(::strerror(errno)));
    }

    m_spec = {0};
    ::timespec ts = Spider::ConvertSecondsToTimespec(m_time);
    Spider::Log_DEBUG("Timer set with: "+Spider::TimespecToString(ts));
    if (m_repeat) {
        m_spec.it_interval = ts;
    }
    m_spec.it_value = ts;
    

    if (timerfd_settime(m_fd, 0, &m_spec, NULL) < 0) {
        throw Spider::SpiderException("Could not set timer!");
    }
    m_expirations = 0;
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


Spider::Return TimerCallback(Spider::TimerHandlePtr timer_p, Spider::Callback cb)
{

    Spider::Log_DEBUG("Starting timeout for fd "+std::to_string(timer_p->GetFD()));

    ReadTimerFd(timer_p);
    Spider::Return ret = cb();
    Spider::Log_DEBUG("Completed timeout for fd "+std::to_string(timer_p->GetFD()));
    if (!timer_p->IsRepeating()) {
        timer_p->Stop();
    }
    return ret;
}



Spider::TimerHandlePtr AddTimer(Spider::Seconds sec, Spider::Callback cb, bool repeat)
{
    Spider::TimerHandlePtr timer_p = nullptr;
    try {
        timer_p = std::make_shared<Spider::TimerHandle>(sec, repeat);
    } catch (Spider::SpiderException &e) {
        Spider::Log_ERROR(e.what());
        return nullptr;
    }

    Spider::Log_DEBUG("Created Timer with FD "+std::to_string(timer_p->GetFD()));

    // Add fd and callback to main spider loop
    try {
        // TODO: Do something with the ID
        Spider::AddFD(timer_p->GetFD(), std::bind(TimerCallback,timer_p,cb));
    } catch(Spider::SpiderException &e) {
        Spider::Log_DEBUG(e.what());
        timer_p.reset();
        return nullptr;
    }

    s_timer_handles[timer_p->GetFD()] = timer_p;

    
    return timer_p;
}


Spider::TimerHandlePtr Spider::CallLater(Spider::Seconds delay, Spider::Callback cb)
{
    return AddTimer(delay, cb, false);
}


Spider::TimerHandlePtr Spider::CallEvery(Spider::Seconds increment, Spider::Callback cb)
{
    return AddTimer(increment, cb, true);
}


uint64_t ReadTimerFd(Spider::TimerHandlePtr t_ptr)
{
    uint64_t expirations = 0;
    if (::read(t_ptr->GetFD(), reinterpret_cast<void*>(&expirations), sizeof(expirations)) < 0) {
        Spider::Log_ERROR("Could not read expirations from timer, exit code "+std::string(::strerror(errno)));
    }
    return expirations;
}