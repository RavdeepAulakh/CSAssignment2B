//
// Created by Ravdeep Aulakh on 2023-11-03.
//

#include "UploadAsyncTask.hpp"
#include <iostream>
#include <thread>
#include <future>

using namespace std;

void UploadAsyncTask::execute() {

    auto future = std::async(&UploadAsyncTask::doInBackground, this);
    future.wait(); // Wait for the background task to complete
    std::string result = future.get();
    onPostExecute(result);
}

void UploadAsyncTask::onPostExecute(const std::string &result) {

    std::cout << result << std::endl;
}

std::string UploadAsyncTask::doInBackground() {

    const string serverAddress = "localhost";
    const int port = 8081;
    const string filePath = "/Users/ravdeepaulakh/Documents/test/a.jpg";

    try {
        uploadClient.postFile(serverAddress, port, "Orange Truck", "2023-10-30", filePath);
        return "File uploaded successfully!";
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return "Error during file upload.";
    }
}

void UploadAsyncTask::onPreExecute() {

    std::cout << "Task is starting..." << std::endl;
}

void UploadAsyncTask::onProgressUpdate(const std::string &progress) {

    std::cout << "Progress: " << progress << std::endl;
}

int main() {
    UploadAsyncTask task;

    // Start the task in a separate thread
    std::future<std::string> result_future = std::async(&UploadAsyncTask::doInBackground, &task);

    std::cout << "Uploading in progress..." << std::endl;

    // Wait for the task to complete and get the result
    std::string result = result_future.get();

    // Process the result
    task.onPostExecute(result);

    return 0;
}