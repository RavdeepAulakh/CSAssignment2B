#include "Activity.hpp"
#include "AsyncTask.hpp"
#include "UploadAsyncTask.hpp"
#include <iostream>

using namespace std;

void Activity::onCreate() {
    AsyncTask* uploadAsyncTask = new UploadAsyncTask();
    uploadAsyncTask->execute();
    std::cout << "Waiting for Callback" << std::endl;
    try {
        std::string input;
        std::cin >> input;
    } catch (const std::exception& e) {
        cout << "Exception occured in Activity.cpp" << endl;
    }
}