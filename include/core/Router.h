#pragma once

#include <unordered_map>
#include <vector>
#include <string>

#include "httpmethod.h"
#include "route.h"

struct Request;
class Response;

class Router {
public:

    void add(HttpMethod method,
             const std::string& path,
             Handler handler);

    void get(const std::string& path, Handler handler);
    void post(const std::string& path, Handler handler);
    void put(const std::string& path, Handler handler);
    void del(const std::string& path, Handler handler);
    void patch(const std::string& path, Handler handler);
    void options(const std::string& path, Handler handler);
    void head(const std::string& path, Handler handler);

    bool dispatch(Request& req, Response& res);

private:
    std::unordered_map<HttpMethod, std::vector<Route>> routes;

    Route buildRoute(const std::string& path,
                     Handler handler);
};