#include "ServerSocket.hpp"
#include <sys/socket.h>
#include <cstring>
#include <unistd.h>
#include <iostream>


// a wrapper class:  utility class used by server.cpp
// to abstract away the lower-level details of socket communication