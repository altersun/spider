#include <exception>


namespace Spider {

// TODO: I have no idea why this works
// https://stackoverflow.com/a/51313801
struct SpiderException: public std::exception
{
    using std::exception::exception;
};


// TODO: Make this a tuple or something
struct Return {
    int ret;
};


// TODO: Make this a tuple or something
struct Input {
    int in;
};


typedef Return (*Callback)(Input);


class FileDescriptor {
    public:
        FileDescriptor(int fid, Callback cb);
        ~FileDescriptor();
        int GetFID();
        int GetSpiderID();
        int Close();
    protected:
        int m_fid;
        int m_spiderid;
        Callback m_callback;
};


class Timer: public FileDescriptor {
    public:
        Timer(float mSec, Callback cb);
        ~Timer();
        float GetTime();
        float GetTimeRemaining();
    protected:
        float m_time;
};


class RepeatTimer: public Timer {
    public:
         Timer(float mSec, Callback cb);
        ~RepeatTimer();
}


} // end namespace Spider