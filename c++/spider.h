#include <errno.h>
#include <functional>
#include <stdexcept>
#include <stdint.h>
#include <string.h>


namespace Spider {


using Return = int; // TODO: determine real return type
using Input = void; // TODO: determine real input type
using Callback = std::function<Return(Input)>;


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



ID AddFD(int fd, Callback callback);
void RemoveFD(int fd);
ID GetID(int fd);

// NOTE: These will run EVERY LOOP. Add with care!
ID AddMaintenanceCall(Callback callback);
ID CallOnce(Callback callback);
void RemoveCall(ID id);

// Convenience functions
int SecondsToTimeout(Seconds seconds);
::timespec ConvertSecondsToTimespec(Seconds seconds);
std::string TimespecToString(::timespec ts);



}; // end Spider namespace 