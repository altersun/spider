# C++ Event Loop
An event loop written in C++ (and some C) that acts on file descriptors

## File Descriptors?
Everything is a file in Linux. Sockets, pipes, actual files...even fun stuff like serial ports and I2C lines. A file "descriptor" (FD) is the Linuxy way of tracking those in C/C++ (they're just ints with a lot of baggage, really). Why not feed these into an event loop?

## Event Loop?
Instead of writing a polling superloop or a pile of threads, let the events of the files drive the sequence of the program! Python has [something similar](https://docs.python.org/3/library/asyncio-eventloop.html) which was the initial inspiration for this project.

## Does this straight up steal from Python?
Only slightly! Especially with the "CallEvery" and "CallLater" funtions, which are directly inspired by the Python event loop. They're implemented with [timerfd](https://man7.org/linux/man-pages/man2/timerfd_create.2.html) which was a lot of fun to learn about.

## So how does it actually work?
The core of the event loop is [epoll](https://man7.org/linux/man-pages/man7/epoll.7.html) with a lot of clean API layered on top. Ultimate plan is to conveniently support multiple types of file descriptors (the aformentioned sockets, I2C, etc.) with plugins like how timers are supported but for now manually adding FDs along with callbacks is the way to go.

## Can this save my marriage?
No, unless your marriage is a polling program being crushed under the weight of it's own internal tracking variables. If not, try open communication and expressing sympathay for your partner's feelings.

## How can I try it out?
Run "make" at this level, then "make run" in the test/ subdirectory.
