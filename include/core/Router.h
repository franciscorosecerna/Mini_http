#pragma once

#include <unordered_map>
#include <vector>
#include <string>

#include "httpmethod.h"
#include "route.h"

struct Request;
class Response;

struct FlatRoute {
    HttpMethod method;
    Route route;
};

class Router {
public:

    struct MountableRoute {
        HttpMethod method;
        std::string path;
        Handler handler;
    };

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

    std::vector<MountableRoute> getMountableRoutes() const;
    const std::vector<FlatRoute>& flatRoutes() const { return flat_; }
private:
    std::unordered_map<HttpMethod, std::vector<Route>> routes;
    std::vector<FlatRoute> flat_;

    Route buildRoute(const std::string& path,
                     Handler handler);
};