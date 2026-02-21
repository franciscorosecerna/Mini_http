#pragma once

#include <string>
#include <vector>
#include <regex>
#include <functional>

namespace mini_http {
    struct Request;
    class Response;

    using Handler = std::function<void(Request&, Response&)>;

    struct Route {
        std::string path;
        std::regex pattern;
        std::vector<std::string> paramNames;
        Handler handler;
    };
}