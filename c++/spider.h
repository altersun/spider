#include <stdexcept>
#include <stdint.h>



namespace Spider {

using SpiderException = std::runtime_error;
using Callback = Return (*)(Input); 

// Handles to running calls
class Handle {
    public:
        Handle();
        void cancel();
        bool cancelled();
        int GetSpiderID();
        virtual ~Handle();
    protected:
        int m_spider_id;
        bool m_cancelled;
};


class TimerHandle : public Handle {
    public:
        TimerHandle();
        float when();
    protected:
        
};

using HandlePtr = std::auto_ptr<Handle>;
using TimerHandlePtr = std::auto_ptr<TimerHandle>;

void SetThreaded(bool threaded);

bool IsRunning();
bool IsThreaded();



// Time-based calls
HandlePtr CallSoon(Callback cb);
TimerHandlePtr CallLater(float delay, Callback cb);
TimerHandlePtr CallAt(float time, Callback cb);
TimerHandlePtr CallEvery(float interval, Callback cb);



template<typename... Ts>
class ThreadHandle: public Handle {
    public:
        ThreadHandle();
        ~ThreadHandle();
        std::future<typename... Ts> GetResult();
};


class EventLoop {
    private:
         EventLoop() {}



};

} // end namespace Spider

namespace Spider::Priority {
    const unsigned int MAX = 128;
    const unsigned int MIN = 0;    

} // end namespace Spider::Priority