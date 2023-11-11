//
// Created by Ravdeep Aulakh on 2023-11-03.
//

#ifndef CSASSIGNMENT2B_ASYNCTASK_HPP
#define CSASSIGNMENT2B_ASYNCTASK_HPP
#include <thread>
#include <iostream>

using namespace std;

class AsyncTask {

public:

    virtual std::string doInBackground() {
        // Placeholder implementation for doInBackground
        return "Default doInBackground result";
    }

    virtual void onPreExecute() {
        // Placeholder implementation for onPreExecute
        std::cout << "Default onPreExecute called" << std::endl;
    }

    virtual void onPostExecute(const std::string& result) {
        // Placeholder implementation for onPostExecute
        std::cout << "Default onPostExecute called with result: " << result << std::endl;
    }

    virtual void onProgressUpdate(const std::string& progress) {
        // Placeholder implementation for onProgressUpdate
        std::cout << "Default onProgressUpdate called with progress: " << progress << std::endl;
    }

    inline void publishProgress(const std::string& progress) {
        onProgressUpdate(progress);
    }

public:

    AsyncTask execute();


};

#endif //CSASSIGNMENT2B_ASYNCTASK_HPP
