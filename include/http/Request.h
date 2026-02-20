#pragma once

#include <string>
#include <unordered_map>
#include "httpmethod.h"

struct Request {
    HttpMethod method;
    std::string path;
    std::string version;
    std::string query;

    std::unordered_map<std::string, std::string> headers;
    std::unordered_map<std::string, std::string> params;

    std::string body;

    bool keepAlive() const {
        auto it = headers.find("connection");

        if (version == "HTTP/1.1") {
            return it == headers.end() || it->second != "close";
        }

        if (version == "HTTP/1.0") {
            return it != headers.end() && it->second == "keep-alive";
        }

        return false;
    }
};