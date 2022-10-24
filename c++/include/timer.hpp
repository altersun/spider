#include <exception>
#include <memory>
#include <stdint.h>

#include "spider.hpp"


namespace Spider {


class TimerHandle : public Handle
{
    public:
        TimerHandle(Seconds s, bool repeat);
        ~TimerHandle();
        Seconds GetAssignedTime();
        Seconds GetTimeRemaining();
        bool IsRepeating();
        void Stop(); // Also serves as cancel for a non-repeating timer
    protected: 
        Seconds m_time;
        ::itimerspec m_spec;
        bool m_repeat;
        uint64_t m_expirations;
};

using TimerHandlePtr = std::shared_ptr<TimerHandle>;


TimerHandlePtr CallEvery(Seconds increment, Callback cb);
TimerHandlePtr CallLater(Seconds delay, Callback cb);







} // end namespace Spider