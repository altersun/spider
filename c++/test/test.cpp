#include <chrono>
#include <iostream>


#include "../include/spider.hpp"
#include "../include/logging.hpp"
#include "../include/timer.hpp"



Spider::Return print_often(Spider::Input)
{
    static uint64_t derp = 0;
    if (derp++%100 == 0) {
        auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        Spider::Log_INFO("HI! "+std::to_string(Spider::GetLoopCount())+" "+std::to_string(now));
    }]['' ]
    return 0;
}


Spider::Return every_cb(Spider::Input)
{
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::cout << "System time " << now << " vs Loop time " << Spider::GetRuntime() << std::endl;
    return 0;
}

int main()
{
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::cout << "System time " << now << std::endl;


    Spider::Log::SetLevel(Spider::Log::DEBUG);
    Spider::Log_INFO("Whatup");

    // CallOnce test 
    Spider::CallOnce([](){Spider::Log_INFO("Testing CallOnce!");return 0;});
    
    // CallEvery test
    auto tp = Spider::CallEvery(2.1, every_cb);
    if (!tp ) {
        Spider::Log_ERROR("CallEvery test setup failed!");
        return -1;
    }
    Spider::Log_INFO("Created CallEvery TimerHandlePointer with fd "+std::to_string(tp->GetFD()));

    // Stop above CallEvery test
    Spider::Callback tp_stopper = [&] {
        Spider::Log_INFO("Stopping timer with fd "+std::to_string(tp->GetFD()));
        tp->Stop();
        return 0;
    };
    if (!Spider::CallLater(5.0, tp_stopper)) {
        Spider::Log_ERROR("Could not set up callback to stop CallEvery TimerHandlerPointer!");
        return -1;
    }

    // CallLater test again, but this time to stop things
    if (!Spider::CallLater(8.0, Spider::Stop)) {
        Spider::Log_ERROR("Could not create exit callback!!!");
        return -1;
    }


    Spider::SetLoopIncrement(0.5);

    Spider::Start();

    Spider::Log_INFO("So long!");

    return 0;
}
