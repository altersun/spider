#include <chrono>
#include <iostream>


#include "spider.h"
#include "timer.h"


Spider::Return every_cb()
{
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::cout << "TIME: " << now << std::endl;
    return 0;
}

int main()
{
    std::cout << "GADS DABNIT!!" << std::endl;



    Spider::CallEvery(2.1, every_cb);
    Spider::CallLater(20.2, Spider::Stop);

    Spider::Start();

    return 0;
}
