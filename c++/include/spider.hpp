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
//using Future = std::future<Return>;


using ID = uint64_t;
using SpiderException = std::runtime_error;
using Seconds = float;

void SetThreaded(bool threaded);

bool IsRunning();
bool IsThreaded();
void Start();
void Start(uint64_t stop_at_event); // TODO: Stop after a specific number of events???
Return Stop(Input);

// Set time in seconds between loop increments
// Endpoint activity will wake the loop but 
// if nothing happens then the loop will wake 
// itself at this increment for maintanence tasks
// Must be a positive non-zero number
void SetLoopIncrement(Seconds seconds);
Seconds GetLoopIncrement();

uint64_t GetLoopCount();
Seconds GetRuntime();


class Handle 
{   
    public:
        Handle(ID id, int fd, Callback callback);
        ~Handle();
        int GetFD();
        ID GetID();
        uint64_t GetActivations();
        Return GetResult();
        bool IsResultReady();
        Callback GetCallback();
    protected:
        ID m_id;
        int m_fd;
        uint64_t m_activations;
        Callback m_callback;
};
using HandlePtr = std::shared_ptr<Handle>;


HandlePtr AddFD(int fd, Callback callback);
void RemoveFD(int fd);
void RemoveFD(HandlePtr hp);
HandlePtr GetHandlePtr(int fd);


// TODO: Create a maintenance call that runs after a provided number of loops
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