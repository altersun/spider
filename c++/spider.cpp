#include <cmath>
#include <queue>
#include <map>
#include <unordered_set>

#include <sys/epoll.h>
#include <sys/time.h>

#include "spider.h"


namespace {

// Spider Types
/*struct fd_entry {
    fd_entry(Spider::ID i, Spider::Callback cb) 
        : id(i)
        , callback(cb)
        {}
    Spider::ID id;
    Spider::Callback callback;
};*/

using fd_entry = std::tuple<Spider::ID, Spider::Callback>;

// Spider variables
bool s_threaded = true;
bool s_running = false;
bool s_stopped = false;
uint64_t s_event_counter = 0;
uint64_t s_stop_at_event_count = 0;
Spider::Seconds s_increment = 0.01;

// epoll variables
const unsigned int MAX_EVENTS = 100;
int s_epoll_fd = -1;
struct epoll_event s_epoll_events[MAX_EVENTS] = {0};


// Map FDs to callbacks
std::map<int, fd_entry> s_fid_map;


} // End anonymous namespace


// // "Private" functions
static void SpiderLoop();
static void AddLoopEvent(int fd);
static void RemoveLoopEvent(int fd);
static void CreateFdWatcher();
static Spider::ID GetNewID();


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


Spider::ID Spider::AddFD(int fd, Spider::Callback callback)
{
    if (s_fid_map.contains(fd)) {
        // TODO: Anthing better than excepting if entry exists?
        throw Spider::SpiderException("Cannot create duplicate entry for "+std::to_string(fd));
    }
    Spider::ID new_id = GetNewID();
    s_fid_map[fd] = fd_entry{new_id, callback};
    AddLoopEvent(fd);
    
    return new_id; 
}


void Spider::RemoveFD(int fd)
{
    if (!s_fid_map.contains(fd)) {
        return;
    }

    s_fid_map.erase(fd);
}


Spider::ID GetID(int fd)
{
    if (!s_fid_map.contains(fd)) {
        return 0;
    }
    return std::get<0>(s_fid_map[fd]);
}


void CreateFdWatcher()
{
    if (s_epoll_fd >= 0) {
        // Already created it!
        // TODO: Throw exception?
        return;
    }
    s_epoll_fd = ::epoll_create1(0);
    if (s_epoll_fd < 0) {
        // TODO: Bail
        throw Spider::SpiderException("Could not create loop");
    }
}


// TODO: Right now this only checks ready for read not using edge triggering
// TODO: Allow for more options
void AddLoopEvent(int fd)
{
    // Cannot continue if epoll has not yet been set up
    if (s_epoll_fd < 0) {
        CreateFdWatcher();
    }

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ::epoll_ctl(s_epoll_fd, EPOLL_CTL_ADD, fd, &ev);
}


void RemoveLoopEvent(int fd)
{
    // Cannot continue if epoll has not yet been set up
    if (s_epoll_fd < 0) {
        return;
    }
    ::epoll_ctl(s_epoll_fd, EPOLL_CTL_DEL, fd, NULL);
}

void SpiderLoop()
{
    // Cannot continue if epoll has not yet been set up
    if (s_epoll_fd < 0) {
        CreateFdWatcher();
    }

    while (s_running) {
        std::queue<Spider::Callback> queued_callbacks;

        // Call epoll, the heart of this whole system
        // TODO: Make a sigmask and fill it out
        int ready = epoll_pwait(s_epoll_fd, s_epoll_events, MAX_EVENTS, 
            Spider::ConvertSecondsToTimeout(Spider::GetLoopIncrement()), NULL);
        if (ready < 0) {
            // TODO: Error handling
        }
        s_event_counter += ready;

        // Epoll management here
        for (int count = 0; count < ready; ++count)
        {
            int fd = s_epoll_events[count].data.fd;
            if (!s_fid_map.contains(fd)) {
                // fd may have been removed while waiting
                // Do not act on it.
                continue;
            }
            queued_callbacks.push(std::get<1>(s_fid_map[fd]));
        }

        // Run through callbacks for ready FDs
        while (!queued_callbacks.empty()) {
            if (s_threaded) {
                // TODO: Thread starts
            } else {
                queued_callbacks.front()();  
            }
            queued_callbacks.pop();
        }


        // TODO: Maintenance stuff
    }

}

// Get a new ID to assign to a new Spider item
Spider::ID GetNewID()
{
    // NOTE: For now just use a monotonically increasing uint
    // TODO: Something more elegant?
    static Spider::ID id = 1;
    Spider::ID ret = id;
    
    // Reserve 0 for errors
    if (++id == 0) {
        id+=1;
    }
    return ret; 
}


void Spider::SetLoopIncrement(Spider::Seconds seconds)
{
    if (seconds <= 0) {
        throw Spider::SpiderException("Loop increment cannot be zero or less!");
    }
    s_increment = seconds;
}


Spider::Seconds Spider::GetLoopIncrement()
{
    return s_increment;
}


int Spider::ConvertSecondsToTimeout(Spider::Seconds seconds)
{
    // epoll timeouts are ints with millisec resolution
    if (seconds <= 0) {
        return 0;
    }
    return static_cast<int>(seconds/1000.0);
}


::timespec Spider::ConvertSecondsToTimespec(Spider::Seconds seconds)
{
    ::timespec ts = {0};
    if (seconds <= 0) {
        return ts;
    }

    ts.tv_nsec = static_cast<long>(100000*std::modf(seconds, &seconds));
    ts.tv_sec = static_cast<time_t>(seconds);
    
    return ts;
}


void Spider::Start(uint64_t stop_at_event = 0)
{
    if (stop_at_event != 0) {
        s_stop_at_event_count = true;
    }
    s_running = true;
    SpiderLoop();
}


void Spider::Start()
{
    Spider::Start(0);
}


int Spider::Stop()
{
    s_running = false;
    return 0;
}