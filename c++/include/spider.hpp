#pragma once
#ifndef SPIDER_HPP
#define SPIDER_HPP

#include <errno.h>
#include <functional>
#include <stdexcept>
#include <stdint.h>
#include <memory>
#include <string.h>


namespace Spider {


using Return = int; // TODO: determine real return type
const Return INVALID_RETURN = -1; // NOTE: This needs to change with return type
using Input = void; // TODO: determine real input type
using Callback = std::function<Return(Input)>;


using ID = uint64_t;
using SpiderException = std::runtime_error;
using Seconds = float;


// Start/Stop/Status
void Start();
void Start(uint64_t stop_at_event); // TODO: Stop after a specific number of events???
Return Stop(Input);
bool IsRunning();

// Set time in seconds between loop increments
// Endpoint activity will wake the loop but 
// if nothing happens then the loop will wake 
// itself at this increment for maintanence tasks
// Must be a positive non-zero number
void SetLoopIncrement(Seconds seconds);
Seconds GetLoopIncrement();

uint64_t GetLoopCount();
Seconds GetRuntime();


// Struct for setting callbacks for more detailed behavior
// Has room to grow with later versions
struct Callbacks
{
    public:
        Callbacks(Callback r_cb, Callback w_cb);
        Callback operator[](int op) const;
        Callback& operator[](int op);
    private:
        Callback m_r_cb;
        Callback m_w_cb;
};


class Handle 
{   
    public:
        Handle(ID id, int fd, Callback callback);
        Handle(ID id, int fd, Callbacks callbacks);
        ~Handle();
        int GetFD();
        ID GetID();
        uint64_t GetActivations();
        const Callback& GetCallback();
        const Callbacks& GetCallbacks();
    protected:
        ID m_id;
        int m_fd;
        uint64_t m_activations;
        Callbacks m_callbacks;
};
using HandlePtr = std::shared_ptr<Handle>;


HandlePtr AddFD(int fd, Callback callback);
HandlePtr AddFD(int fd, Callbacks callbacks);
void RemoveFD(int fd);
void RemoveFD(HandlePtr hp);
HandlePtr GetHandlePtr(int fd);


// Create a maintenance call that runs after a provided number of loops
// TODO: ID AddMaintenanceCall(Callback callback, size_t after_loops)
// NOTE: These will run EVERY LOOP. Add with care!
ID AddMaintenanceCall(Callback callback);
ID CallOnce(Callback callback);
void RemoveCall(ID id);

// Convenience functions
int SecondsToTimeout(Seconds seconds);
::timespec ConvertSecondsToTimespec(Seconds seconds);
std::string TimespecToString(::timespec ts);

// Gets next ID in sequence
// Intended to be used by plugins and not by a user
// TODO: Maybe put this in a private header?
ID GetNextID();
ID AddFD(HandlePtr hp); // Also should only be used by plugins. Returns correct ID on success.


}; // end Spider namespace 

#endif // SPIDER_HPP