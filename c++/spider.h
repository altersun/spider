#include <stdexcept>
#include <stdint.h>



namespace Spider {

using ID = uint64_t;
using SpiderException = std::runtime_error;
using Callback = Return (*)(Input);
using Seconds = float;






void SetThreaded(bool threaded);

bool IsRunning();
bool IsThreaded();
void Start();
void Start(uint64_t stop_at_event); // TODO: Stop after a specific number of events???
void Stop();

// Set time in seconds between loop increments
// Endpoint activity will wake the loop but 
// if nothing happens then the loop will wake 
// itself at this increment for maintanence tasks
// Must be a positive non-zero number
void SetLoopIncrement(Seconds seconds);
Seconds GetLoopIncrement();

ID AddFD(int fd, Callback callback);
ID GetID(int fd);




}; // end Spider namespace 