#include <cstring> // For strerror
#include <fcntl.h> // For file modes
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "spider.hpp"
#include "timer.hpp"


static void print_help();

int main(int argc, char* argv[])
{
    int opt;
    std::string pipename;

    while ((opt = getopt(argc, argv, "hp:")) != -1) {
        switch (opt) {
            case 'h': {
                print_help();
                break;
            } case 'p': {
                pipename = std::string(optarg);
                break;
            } default: {
                throw Spider::SpiderException("Bad argument, please read the help and try again");
            }
        }
    }
    


    // Create the fifo and open it
    if (0 != mkfifo(pipename.c_str(), 0666)) {
        throw Spider::SpiderException("Could not create named pipe " + pipename + std::string(strerror(errno)));
    }
    int pipe_fd = open(pipename.c_str(), O_RDONLY | O_NONBLOCK);
    if (pipe_fd <= 0) {
        throw Spider::SpiderException("Could not open named pipe " + pipename + std::string(strerror(errno)));
    }


    // Spider Setup
    Spider::TimerHandlePtr stopper = Spider::CallLater(10.0, Spider::Stop);
    Spider::Callback read_cb = [&]() {
        // We recieved data so reset the stopper
        stopper->Stop();

        // TODO: Read fifo
    };


}


void print_help()
{
    // TODO: Do for real
    std::cout << "Help!" << std::endl;
}