#include <exception>
#include <memory>
#include <stdint.h>

#include "spider.h"


namespace Spider {


class TimerHandle
{
    public:
        TimerHandle(Seconds s, bool repeat);
        ~TimerHandle();
        Seconds GetAssignedTime();
        Seconds GetTimeRemaining();
        bool IsRepeating();
        int GetFD();
        int GetID();
        void Stop(); // Also serves as cancel for a non-repeating timer
    private: 
        Seconds m_time;
        ::itimerspec m_spec;
        bool m_repeat;
        int m_fd;
        ID m_id;
        uint64_t m_expirations;
};

using TimerHandlePtr = std::shared_ptr<TimerHandle>;


TimerHandlePtr CallEvery(Seconds increment, Callback cb);
TimerHandlePtr CallLater(Seconds delay, Callback cb);







} // end namespace Spider