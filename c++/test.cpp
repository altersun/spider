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
    std::cout << "GADS DABNIT!!" << std::endl;
    Spider::Log_INFO("Whatup");


    Spider::CallEvery(2.1, every_cb);
    Spider::CallLater(20.2, Spider::Stop);
    Spider::AddMaintenanceCall(print_often);
    //Spider::SetLoopIncrement(0.1);
    
    Spider::Log_INFO("Hell yeah");

    Spider::Start();

    return 0;
}
