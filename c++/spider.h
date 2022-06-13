#include <stdexcept>
#include <stdint.h>



namespace Spider {


using Return = int; // TODO: determine real return type
using Input = void; // TODO: determine real input type
using Callback = Return (*)(Input);
//using Callback = Return (*)();

using ID = uint64_t;
using SpiderException = std::runtime_error;
using Seconds = float;






void SetThreaded(bool threaded);

bool IsRunning();
bool IsThreaded();
void Start();
void Start(uint64_t stop_at_event); // TODO: Stop after a specific number of events???
int Stop();

// Set time in seconds between loop increments
// Endpoint activity will wake the loop but 
// if nothing happens then the loop will wake 
// itself at this increment for maintanence tasks
// Must be a positive non-zero number
void SetLoopIncrement(Seconds seconds);
Seconds GetLoopIncrement();

ID AddFD(int fd, Callback callback);
void RemoveFD(int fd);
ID GetID(int fd);

// Convenience functions
int ConvertSecondsToTimeout(Seconds seconds);
::timespec ConvertSecondsToTimespec(Seconds seconds);



}; // end Spider namespace 