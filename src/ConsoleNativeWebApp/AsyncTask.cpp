// Java's AsyncTask but in C++ using POSIX threads

// responsible for asynchronously uploading files
#include "AsyncTask.hpp";
#include <thread>
#include <iostream>

using namespace std;

AsyncTask AsyncTask::execute() {

    onPreExecute();
    std::thread(&AsyncTask::doInBackground, this).detach();
    return *this;
}