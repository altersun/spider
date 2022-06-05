#include <exception>
#include <memory>

#include <spider.h>


namespace Spider {




// TODO: Make this a tuple or something
struct Return {
    int ret;
};


// TODO: Make this a tuple or something
struct Input {
    int in;
};


//typedef Return (*Callback)(Input);





class Event {
    public:  
        virtual ~Event();
        int GetFID();
        int GetSpiderID();
        Callback GetCallback();
        Return operator()(Input);
        virtual void Close() = 0;

        // Not public Constructor
        // Force caller to make a shared_ptr
        template <class ...Args>
        friend std::shared_ptr<Event> make(Args&& ...args)
        {
             std::shared_ptr<Event> ptr = new Event(std::forward<Args>(args)...);

        }

    protected:
        Event() = default;
        int m_fid;
        int m_spiderid;
        Callback m_callback;
};


using EventPtr = std::shared_ptr<Event>;


class Timer: public Event {
    public:
        ~Timer();
        float GetTime();

        /**
         * Get time remaining before timeout
         * return: time on success, negative value on failure
         */
        float GetTimeRemaining();
        bool IsRepeating();
        void StopRepeating();
        void Close();
    protected:
        Timer(Callback cb, float mSec, bool repeat);
        void SetTimeRemaining(float seconds);
        float m_seconds;
        bool m_repeat;
};





} // end namespace Spider