//
// Created by Ravdeep Aulakh on 2023-11-03.
//

#ifndef CSASSIGNMENT2B_FILEUPLOADEXCEPTION_HPP
#define CSASSIGNMENT2B_FILEUPLOADEXCEPTION_HPP

#include <stdexcept>

class FileUploadException : public std::runtime_error {
public:
    FileUploadException(const std::string& message) : std::runtime_error(message) {}
};

#endif //CSASSIGNMENT2B_FILEUPLOADEXCEPTION_HPP
