cmake_minimum_required(VERSION 3.26)
project(CSAssignment2B)

set(CMAKE_CXX_STANDARD 14)

include_directories(src/ConsoleNativeWebApp)

add_executable(CSAssignment2BConsole

        UploadClient.cpp
        Socket.cpp
        Socket.hpp
        FileUploadException.hpp
        UploadAsyncTask.cpp
        UploadAsyncTask.hpp
        Activity.cpp
        Activity.hpp
        WorkerThread.hpp
        AsyncTask.cpp
        AsyncTask.hpp
)