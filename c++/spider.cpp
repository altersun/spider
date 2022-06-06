#include <unordered_map>
#include <unordered_set>

#include <sys/epoll.h>
#include <sys/time.h>

#include <spider.h>
//#include <file_handler.h>

namespace {

// Spider Types
struct fd_entry {
    Spider::ID id;
    Spider::Callback callback;
    fd_entry(Spider::ID id, Spider::Callback callback) 
        : id(id)
        , callback(callback)
        {}
};

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
std::unordered_map<int, fd_entry> s_fid_map;


// Iterate over these after waking up
std::unordered_map<uint64_t, Spider::Callback> s_soon_callbacks;

// Presently running threads
//std::unordered_map<uint64_t, TODO_thread_type> s_active_threads;

// "Private" functions
void SpiderLoop();
void AddLoopEvent(int fd);
void CreateFdWatcher();
Spider::ID GetNewID();

// Private conversion functions
int ConvertSecondsToTimeout(Spider::Seconds);
::timespec ConvertSecondsToTimespec(Spider::Seconds);


} // End anonymous namespace


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


Spider::ID AddFD(int fd, Spider::Callback callback)
{
    if (s_fid_map.contains(fd)) {
        // TODO: Anthing better than excepting if entry exists?
        throw Spider::SpiderException("Cannot create duplicate entry for "+std::to_string(fd));
    }
    s_fid_map[fd] = fd_entry(GetNewID(), callback);
    AddLoopEvent(fd);
}


Spider::ID GetID(int fd)
{
    if (!s_fid_map.contains(fd)) {
        return 0;
    }
    return s_fid_map[fd].id;
}


void CreateFdWatcher()
{
    if (s_epoll_fd >= 0) {
        // Already created it!
        // TODO: Throw exception?
        return;
    }
    s_epoll_fd = ::epoll_create1(0);
    if (s_epoll_events != 0) {
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


void SpiderLoop()
{
    // Cannot continue if epoll has not yet been set up
    if (s_epoll_fd < 0) {
        CreateFdWatcher();
    }

    while (s_running) {}
        // Call epoll, the heart of this whole system
        // TODO: Make a sigmask and fill it out
        int ready = epoll_pwait(s_epoll_fd, s_epoll_events, MAX_EVENTS, ConvertSecondsToTimeout(Spider::GetLoopIncrement()), NULL);
        if (ready < 0) {
            // TODO: Error handling
        }
        s_event_counter += ready;

        // Epoll management here


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


int ConvertSecondsToTimeout(Spider::Seconds seconds)
{
    // epoll timeouts are ints with millisec resolution
    if (seconds <= 0) {
        return 0;
    }
    return static_cast<int>(seconds/1000.0);
}


::timespec ConvertSecondsToTimespec(Spider::Seconds seconds)
{
    ::timespec t = {0};
    if (seconds <= 0) {
        return t;
    }
    
    // TODO: Implement for real
    return t;
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


void Spider::Stop()
{
    s_running = false;
}