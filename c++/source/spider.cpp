#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <future>
#include <queue>
#include <map>
#include <unordered_map>

#include <sys/epoll.h>
#include <sys/time.h>

#include "../CTPL/ctpl_stl.h"

#include "../include/spider.hpp"
#include "../include/logging.hpp"


// TODO: Put some thought into this
#define DEFAULT_THREADS (16)


namespace {

using fd_entry = std::tuple<Spider::ID, Spider::Callback>;

// Spider variables
bool s_threaded = false;
std::unique_ptr<ctpl::thread_pool> s_threadpool_ptr = nullptr;
std::atomic_bool s_running = false;
std::atomic_bool s_stopped = false;
std::atomic<uint64_t> s_event_counter = 0;
uint64_t s_stop_at_event_count = 0;
std::atomic<uint64_t> s_loop_counter = 0;
Spider::Seconds s_increment = 0.01;
std::chrono::time_point<std::chrono::steady_clock> s_starttime;


// epoll variables
const unsigned int MAX_EVENTS = 100;
int s_epoll_fd = -1;
struct epoll_event s_epoll_events[MAX_EVENTS] = {0};


// Map FDs to Handle objects
std::unordered_map<int, Spider::HandlePtr> s_handle_map;

// The futures that store the results of callbacks are in their own structure for privacy
// from the user. They are linked to their associated Handle objects by ID number
std::unordered_map<Spider::ID, std::future<Spider::Return>> s_future_map;

// Map Spider IDs to maintenance functions
std::unordered_map<Spider::ID, Spider::Callback> s_maintenance_map;

// Map Spider IDs to one-off callbacks
std::unordered_map<Spider::ID, Spider::Callback> s_once_map;

} // End anonymous namespace


// "Private" functions
static void SpiderLoop();
static void AddLoopEvent(int fd);
static void RemoveLoopEvent(int fd);
static void CreateFdWatcher();



Spider::Handle::Handle(Spider::ID id, int fd, Spider::Callback callback)
 : m_id(id)
 , m_fd(fd)
 , m_activations(0)
 , m_callback(callback)
{
} 


Spider::Handle::~Handle()
{
    if (s_future_map.count(GetID()) <= 0) {
        s_future_map.erase(GetID());
    }
}


int Spider::Handle::GetFD()
{
    return m_fd;
}


Spider::ID Spider::Handle::GetID()
{
    return m_id;
}


uint64_t Spider::Handle::GetActivations()
{
    return m_activations;
}


Spider::Return Spider::Handle::GetResult()
{
    if (IsResultReady()) {
        return s_future_map[GetID()].get();
    }
    return Spider::INVALID_RETURN;
}    


bool Spider::Handle::IsResultReady()
{
    return s_future_map[GetID()].valid();
}


Spider::Callback Spider::Handle::GetCallback()
{
    return m_callback;
}


bool Spider::IsRunning()
{
    return s_running;
}


bool Spider::IsThreaded()
{
    return s_threadpool_ptr != nullptr;
}


void Spider::SetThreaded(bool threaded)
{
    if (Spider::IsRunning()) {
        throw Spider::SpiderException("Cannot set threading policy while running");
    }
    
    if (threaded && !s_threadpool_ptr) {
        // TODO: Make it possible to set amount of threads here
        s_threadpool_ptr = std::make_unique<ctpl::thread_pool>(DEFAULT_THREADS);
    } else if (!threaded && s_threadpool_ptr) {
        s_threadpool_ptr.reset(nullptr);
    }
}


Spider::HandlePtr AddFD(int fd, Spider::Callback callback)
{
    if (s_handle_map.count(fd) > 0) {
        // TODO: Anthing better than excepting if entry exists?
        throw Spider::SpiderException("Cannot create duplicate entry for "+std::to_string(fd));
    }
    if (callback == nullptr) {
        throw Spider::SpiderException("Cannot register a null callback for "+std::to_string(fd));
    }
    
    s_handle_map[fd] = std::make_shared<Spider::Handle>(Spider::GetNextID(), fd, callback); 
    AddLoopEvent(fd);
    
    return s_handle_map[fd]; 
}


Spider::ID AddFD(Spider::HandlePtr hp)
{
    if (s_handle_map.count(hp->GetFD()) > 0) {
        // TODO: Anthing better than excepting if entry exists?
        throw Spider::SpiderException("Cannot create duplicate entry for "+std::to_string(hp->GetFD()));
    }
    if (hp->GetCallback() == nullptr) {
        throw Spider::SpiderException("Cannot register a null callback for "+std::to_string(hp->GetFD()));
    }
    
    s_handle_map[hp->GetFD()] = hp; 
    AddLoopEvent(hp->GetFD());
    
    return hp->GetID();
}


void Spider::RemoveFD(int fd)
{
    if (!(s_handle_map.count(fd) > 0)) {
        return;
    }

    RemoveLoopEvent(fd);
    s_handle_map.erase(fd);
}


void Spider::RemoveFD(Spider::HandlePtr hp)
{
    int fd = hp->GetFD(); // In case pointer gets deleted
    Spider::RemoveFD(fd);
    /* Old inefficient way left here as a map iteration example for self
    for (const auto& [fd, handle_ptr] : s_handle_map) {
        if (hp == handle_ptr) {
            RemoveFD(fd);
            break;
        }
    }
    */
}


Spider::HandlePtr GetHandlePtr(int fd)
{
    if (!(s_handle_map.count(fd) > 0)) {
        return 0;
    }
    return s_handle_map[fd];
}


void CreateFdWatcher()
{
    if (s_epoll_fd >= 0) {
        // Already created it!
        // TODO: Throw exception?
        Spider::Log::Log_Basic(Spider::Log::ERROR, "Cannot create epoll!");
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
    
    Spider::Log_DEBUG("Added fd "+std::to_string(fd));
    struct epoll_event ev = {0};
    ev.events = EPOLLIN;
    ev.data.fd = fd;
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

    s_starttime = std::chrono::steady_clock::now();

    while (s_running) {
        std::queue<Spider::HandlePtr> queued_callbacks;

        // Call epoll, the heart of this whole system
        // TODO: Make a sigmask and fill it out
        int timeout = Spider::SecondsToTimeout(Spider::GetLoopIncrement());
        Spider::Log_DEBUG("Waiting with delay "+std::to_string(timeout)+ " at time "+std::to_string(Spider::GetRuntime()));
        int ready = epoll_pwait(s_epoll_fd, s_epoll_events, MAX_EVENTS, 
            timeout, NULL);
        if (ready < 0) {
            // TODO: Error handling
        } else if (ready > 0) { 
        
            Spider::Log_DEBUG("FDs ready: "+std::to_string(ready));
            s_event_counter += ready;

            // Epoll management here
            for (int count = 0; count < ready; count++) {
                int fd = s_epoll_events[count].data.fd;
                Spider::Log_DEBUG("Handling ready fd "+std::to_string(fd));
                if (s_handle_map.count(fd) == 0) {
                    // fd may have been removed while waiting
                    // Do not act on it.
                    continue;
                }
                if (s_handle_map[fd] == nullptr) {
                    Spider::Log_ERROR("Retrieved null callback for fd "+std::to_string(fd));
                    continue;
                }
                queued_callbacks.push(s_handle_map[fd]);
            }

            // Run through callbacks for ready FDs
            while (!queued_callbacks.empty()) {
                auto to_call = queued_callbacks.front();
                if (s_threaded) {
                    // TODO: Thread starts
                    
                } else {
                    try {
                        s_future_map[to_call->GetID()] = std::async(std::launch::deferred, to_call->GetCallback());
                    } catch (const std::bad_function_call& b) {
                        Spider::Log_ERROR("Could not run callback: "+std::string(b.what()));
                    }
                }
                queued_callbacks.pop();
            }
        }

        // Maintenance stuff
        for (const auto& [ID, callback] : s_maintenance_map) {
            // TODO: Threading?
            callback();
        }

        // One-offs
        for (const auto& [id, callback] : s_once_map) {
            // TODO: Threading?
            callback();
        }
        s_once_map.clear();

        s_loop_counter++;
    }
}

// Get a new ID to assign to a new Spider item
Spider::ID GetNextID()
{
    // NOTE: For now just use a monotonically increasing uint
    // TODO: Something more elegant?
    static std::atomic<Spider::ID> id = 0;
    
    // Reserve 0 for errors
    if (++id == 0) {
        id+=1;
    }
    return id; 
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


int Spider::SecondsToTimeout(Spider::Seconds seconds)
{
    // epoll timeouts are ints with millisec resolution
    if (seconds <= 0) {
        return 0;
    }
    return static_cast<int>(seconds*1000.0);
}


::timespec Spider::ConvertSecondsToTimespec(Spider::Seconds seconds)
{
    ::timespec ts = {0};
    if (seconds <= 0) {
        return ts;
    }

    ts.tv_nsec = static_cast<long>(::pow(10.0, 9.0)*std::modf(seconds, &seconds));
    ts.tv_sec = static_cast<time_t>(seconds);
    
    return ts;
}


void Spider::Start(uint64_t stop_at_event = 0)
{
    if (stop_at_event != 0) {
        s_stop_at_event_count = true;
    }
    s_running = true;
    Spider::Log::Log_Basic(Spider::Log::INFO, "Started poll");
    SpiderLoop();
}


void Spider::Start()
{
    Spider::Start(0);
}


Spider::Return Spider::Stop(Spider::Input)
{
    Spider::Log_DEBUG("Stopping loop after runtime (seconds): "+std::to_string(Spider::GetRuntime()));
    s_running = false;
    if (s_threadpool_ptr) {
        s_threadpool_ptr->stop();
    }
    return 0;
}


uint64_t Spider::GetLoopCount()
{
    return s_loop_counter;
}


Spider::Seconds Spider::GetRuntime()
{
    if (!s_running) {
        return 0.0;
    }
    return std::chrono::duration<Spider::Seconds>(std::chrono::steady_clock::now()-s_starttime).count();
}


Spider::ID Spider::AddMaintenanceCall(Spider::Callback callback) {
    Spider::ID id = GetNextID();
    s_maintenance_map[id] = callback;
    return id;
}


Spider::ID Spider::CallOnce(Spider::Callback callback) {
    Spider::ID id = GetNextID();
    s_once_map[id] = callback;
    return id;
}


void Spider::RemoveCall(Spider::ID id)
{
    if (s_maintenance_map.count(id) > 0) {
        s_maintenance_map.erase(id);
    } else if (s_once_map.count(id) > 0) {
        s_once_map.erase(id);
    }
}


std::string Spider::TimespecToString(::timespec ts)
{
    static const size_t NSEC_LEN = 9;
    auto raw_nsec = std::to_string(ts.tv_nsec);
    auto nsec = std::string(NSEC_LEN - std::min(NSEC_LEN, raw_nsec.length()), '0') + raw_nsec;
    return std::to_string(ts.tv_sec) + "." + nsec;
}