#include <exception>
#include <memory>

#include <spider.h>


namespace Spider {


class TimerHandle
{
    public:
        TimerHandle(Seconds s, bool repeat);
        Seconds GetAssignedTime();
        Seconds GetTimeRemaing();
        bool IsRepeating();
        void Stop(); // Also serves as cancel for a non-repeating timer
    private: 
        Seconds m_time;
        bool m_repeat;
        int m_fd;
};



TimerHandle CallLater(Seconds delay, Callback cb);
TimerHandle CallEvery(Seconds increment, Callback cb);






} // end namespace Spider