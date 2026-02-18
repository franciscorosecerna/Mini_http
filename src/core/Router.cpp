#include "../include/core/Router.h"
#include "../include/core/Request.h"
#include "../include/core/Response.h"

void Router::add(HttpMethod method,
                 const std::string& path,
                 Handler handler)
{
    routes[method].push_back(
        buildRoute(path, std::move(handler))
    );
}

void Router::get(const std::string& path, Handler handler) {
    add(HttpMethod::GET, path, std::move(handler));
}

void Router::post(const std::string& path, Handler handler) {
    add(HttpMethod::POST, path, std::move(handler));
}

void Router::put(const std::string& path, Handler handler) {
    add(HttpMethod::PUT, path, std::move(handler));
}

void Router::del(const std::string& path, Handler handler) {
    add(HttpMethod::DELETE_, path, std::move(handler));
}

void Router::patch(const std::string& path, Handler handler) {
    add(HttpMethod::PATCH, path, std::move(handler));
}

void Router::options(const std::string& path, Handler handler) {
    add(HttpMethod::OPTIONS, path, std::move(handler));
}

void Router::head(const std::string& path, Handler handler) {
    add(HttpMethod::HEAD, path, std::move(handler));
}

bool Router::dispatch(Request& req, Response& res)
{
    auto it = routes.find(req.method);
    if (it == routes.end())
        return false;

    for (auto& route : it->second) {
        std::smatch match;

        if (std::regex_match(req.path, match, route.pattern)) {

            req.params.clear();

            for (size_t i = 0; i < route.paramNames.size(); ++i) {
                req.params[route.paramNames[i]] =
                    match[i + 1].str();
            }

            route.handler(req, res);
            return true;
        }
    }

    return false;
}

Route Router::buildRoute(const std::string& path,
                         Handler handler)
{
    Route route;
    route.path = path;
    route.handler = std::move(handler);

    std::string regexStr;
    size_t i = 0;

    while (i < path.size()) {
        if (path[i] == ':') {
            ++i;
            std::string paramName;

            while (i < path.size() && path[i] != '/') {
                paramName += path[i++];
            }

            route.paramNames.push_back(paramName);
            regexStr += "([^/]+)";
        }
        else if (path[i] == '*') {
            route.paramNames.push_back("wildcard");
            regexStr += "(.*)";
            ++i;
        }
        else {
            char c = path[i++];

            if (std::string(".+?{}[]()^$|\\")
                .find(c) != std::string::npos)
            {
                regexStr += '\\';
            }

            regexStr += c;
        }
    }

    route.pattern = std::regex(
        "^" + regexStr + "$",
        std::regex_constants::ECMAScript |
        std::regex_constants::optimize
    );

    return route;
}