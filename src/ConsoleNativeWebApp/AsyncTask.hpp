//
// Created by Ravdeep Aulakh on 2023-11-03.
//

#ifndef CSASSIGNMENT2B_ASYNCTASK_HPP
#define CSASSIGNMENT2B_ASYNCTASK_HPP
#include <thread>
#include <iostream>

using namespace std;

class AsyncTask {

protected:

    virtual string doInBackground();

    virtual void onPreExecute();

    virtual void onPostExecute(const std::string& result);

    virtual void onProgressUpdate(const std::string& progress);

    inline void publishProgress(const std::string& progress) {
        onProgressUpdate(progress);
    }

public:

    AsyncTask execute();


};

#endif //CSASSIGNMENT2B_ASYNCTASK_HPP
