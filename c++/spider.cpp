#include <unordered_map>

#include <spider.h>
#include <file_handler.h>

namespace {

bool s_threaded = true;
bool s_running = false;

std::unordered_map<int, Spider::EventPtr> s_fid_map;


}


bool Spider::IsRunning()
{
    return s_running;
}


bool Spider::IsThreaded()
{
    return s_threaded;
}


void Spider::SetThreaded(bool threaded)
{
    if (Spider::IsRunning()) {
        throw Spider::SpiderException("Cannot set threading policy while running");
    }
    s_threaded = threaded;
}