// manages worker threads
#pragma once
#include "AsyncTask.hpp"
#include <thread>
#include <iostream>

class WorkerThread {
public:
    WorkerThread(AsyncTask* asyncTask) : aTask(asyncTask) {}

    void run() {
        std::string result = aTask->doInBackground();
        aTask->onPostExecute(result);
    }

private:
    AsyncTask* aTask;
};
