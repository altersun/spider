#include



namespace Spider {

// Priorities
//static const unsigned int  





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