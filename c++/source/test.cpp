#include <chrono>
#include <iostream>


#include "spider.hpp"
#include "logging.hpp"
#include "timer.hpp"



Spider::Return print_often(Spider::Input)
{
    static uint64_t derp = 0;
    if (derp++%100 == 0) {
        auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        Spider::Log_INFO("HI! "+std::to_string(Spider::GetLoopCount())+" "+std::to_string(now));
    }
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
    Spider::Log::SetLevel(Spider::Log::INFO);
    Spider::Log_INFO("Whatup");


    Spider::CallOnce([](){Spider::Log_INFO("Testing CallOnce!");return 0;});
    
    // CallEvery test
    auto tp = Spider::CallEvery(2.1, every_cb);
    if (!tp ) {
        Spider::Log_ERROR("CallEvery test setup failed!");
        return -1;
    }

    // CallOnce test 
    Spider::Callback once_test = [&] {
        Spider::Log_INFO("Stopping timer with fd "+std::to_string(tp->GetFD()));
        tp->Stop();
        return 0;
    };
    if (!Spider::CallLater(5.0, once_test)) {
        Spider::Log_ERROR("CallLater 2 test setup failed!");
        return -1;
    }

    // CallLater test again, but this time to stop things
    if (!Spider::CallLater(8.0, Spider::Stop)) {
        Spider::Log_ERROR("Could not create exit callback!!!");
        return -1;
    }


    Spider::SetLoopIncrement(0.05);

    Spider::Start();

    Spider::Log_INFO("So long!");

    return 0;
}
