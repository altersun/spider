#include <unordered_map>
#include <unordered_set>

#include <sys/epoll.h>

#include <spider.h>
#include <file_handler.h>

namespace {

// Spider variables
bool s_threaded = true;
bool s_running = false;
uint64_t s_spider_id = 0;

// epoll variables
const unsigned int MAX_EVENTS = 100;
int s_epoll_fd = -1;
struct epoll_event s_epoll_events[MAX_EVENTS] = {0};


std::unordered_map<int, Spider::EventPtr> s_fid_map;

// Iterate over these after waking up
std::unordered_map<uint64_t, Spider::Callback> s_soon_callbacks;

// Presently running threads
//std::unordered_map<uint64_t, TODO_thread_type> s_active_threads;

// "Private" functions
void SpiderLoop();
void AddLoopEvent(int fd);

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


Spider::HandlePtr Spider::MakeHandle(int id)
{
    return HandlePtr(new Spider::Handle(id));
} 

Spider::HandlePtr Spider::CallSoon(Spider::Callback cb)
{

}

// TODO: Right now this only checks ready for read not using edge triggering
// TODO: Allow for more options
void AddLoopEvent(int fd)
{
    // Cannot continue if epoll has not yet been set up
    if (s_epoll_fd < 0) {
        // TODO: Exception?
        return;
    }

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ::epoll_ctl(s_epoll_fd, EPOLL_CTL_ADD, fd, &ev);
}


void Spider::Handle::cancel()
{
    
}


void SpiderLoop()
{
    s_epoll_fd = ::epoll_create1(0);
    if (s_epoll_events != 0) {
        // TODO: Bail
        throw Spider::SpiderException("Could not create loop");
    }
    
    for (auto& [fd, unused]: s_fid_map) {
        AddLoopEvent(fd);
    }

    // TODO: Epoll wait

    for (auto& [id, callback]: s_soon_callbacks) {
        // TODO: Start thread with id, callback
        s_soon_callbacks.erase(id);
    }

}