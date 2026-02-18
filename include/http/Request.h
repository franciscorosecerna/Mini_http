#pragma once

#include <string>
#include <unordered_map>
#include "httpmethod.h"

struct Request {
    HttpMethod method;
    std::string path;
    std::unordered_map<std::string, std::string> params;
};