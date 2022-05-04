#include <cmath>
#include <sys/time.h>
#include <sys/timerfd.h>
#include <unistd.h>

#include <spider.h>
#include <file_handler.h>


Spider::Event::~Event()
{
    Close();
}


int Spider::Event::GetFID() 
{
    return m_fid;
}


int Spider::Event::GetSpiderID()
{
    return m_spiderid;
}


Spider::Callback Spider::Event::GetCallback()
{
    return m_callback;
}


Spider::Return Spider::Event::operator()(Spider::Input input)
{
    return m_callback(input);
}



Spider::Timer::Timer(Callback cb, float seconds, bool repeat) 
    : m_seconds(seconds) 
    , m_repeat(repeat)
{
    // TODO: Make this #define to use CLOCK_BOOTTIME if ALARM not supported
    // TODO: See if the TFD_NONBLOCK flag is needed for the 2nd argument
    // https://man7.org/linux/man-pages/man2/timerfd_create.2.html
    int m_fid = timerfd_create(CLOCK_BOOTTIME_ALARM, 0);
    if (m_fid == -1) {
        throw Spider::SpiderException("Could not obtain timer!");
    }

    SetTimeRemaining(seconds);
}


void Spider::Timer::SetTimeRemaining(float seconds)
{
    timespec ts = {0};
    itimerspec interval = {0};

    ts.tv_nsec = static_cast<long>(100000*std::modf(seconds, &seconds));
    ts.tv_sec = static_cast<time_t>(seconds);

    if (m_repeat) {
        interval.it_interval = ts;
    } else {
        interval.it_value = ts;
    }

    timerfd_settime(m_fid, 0, &interval, NULL);
}


float Spider::Timer::GetTime()
{
    return m_seconds;
}


float Spider::Timer::GetTimeRemaining()
{
    itimerspec interval = {0};
    int ret = timerfd_gettime(m_fid, &interval);
    if (ret != 0) {
        // TODO: Logging of what went wrong
        return -1.0;
    }
    return static_cast<float>(interval.it_value.tv_sec + interval.it_value.tv_nsec*100000);
}


bool Spider::Timer::IsRepeating()
{
    return m_repeat;
}


void Spider::Timer::StopRepeating()
{
    if (!m_repeat) {
        return;
    }

    m_repeat == false;
    float time_remaining = GetTimeRemaining();
    SetTimeRemaining(time_remaining);
}


void Spider::Timer::Close()
{
    ::close(m_fid);
}