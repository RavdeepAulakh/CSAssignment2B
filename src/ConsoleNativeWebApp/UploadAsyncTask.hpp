//
// Created by Ravdeep Aulakh on 2023-11-03.
//

#ifndef CSASSIGNMENT2B_UPLOADASYNCTASK_HPP
#define CSASSIGNMENT2B_UPLOADASYNCTASK_HPP
#include <iostream>
#include "UploadClient.cpp"
#include "AsyncTask.hpp"

using namespace std;
class UploadAsyncTask : public AsyncTask{

public:

    void execute();

    void onPostExecute(const std::string& result) override;

    std::string doInBackground() override;

    virtual void onPreExecute() override;

    virtual void onProgressUpdate(const std::string& progress) override;

private:

    UploadClient uploadClient;
};

#endif //CSASSIGNMENT2B_UPLOADASYNCTASK_HPP
