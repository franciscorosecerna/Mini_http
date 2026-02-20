#pragma once

#include <functional>
#include "Router.h"
#include "Middleware.h"
#include "TcpServer.h"
#include "Request.h"
#include "Response.h"
#include "HttpParser.h"

class App {
public:
    using Handler = std::function<void(Request&, Response&)>;

    App(size_t threads = 4);

    void get(const std::string& path, Handler handler);
    void post(const std::string& path, Handler handler);
    void put(const std::string& path, Handler handler);
    void del(const std::string& path, Handler handler);
    void patch(const std::string& path, Handler handler);
    void options(const std::string& path, Handler handler);
    void head(const std::string& path, Handler handler);

    void use(Middleware middleware);

    void listen(int port);

private:
    size_t count;
    Router router;
    MiddlewareChain middlewareChain;
    std::unique_ptr<TcpServer> server;

    bool handleClient(Connection& conn);
};