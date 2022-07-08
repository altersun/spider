#include <chrono>
#include <iostream>


#include "spider.h"
#include "logging.h"
#include "timer.h"



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
    std::cout << "TIME: " << now << std::endl;
    return 0;
}

int main()
{
    Spider::Log::SetLevel(Spider::Log::DEBUG);
    Spider::Log_INFO("Whatup");


    Spider::CallOnce(every_cb);
    Spider::CallEvery(2.1, every_cb);
    /*if (!Spider::CallLater(3.0, every_cb)) {
        Spider::Log_ERROR("Could not create simple callback!!!");
        return -1;
    }*/

    if (!Spider::CallLater(3.0, Spider::Stop)) {
        Spider::Log_ERROR("Could not create exit callback!!!");
        return -1;
    }
    //Spider::AddMaintenanceCall(print_often);
    Spider::SetLoopIncrement(1.0);
    
    Spider::Log_INFO("Hell yeah");

    Spider::Start();

    Spider::Log_INFO("So long!");

    return 0;
}
