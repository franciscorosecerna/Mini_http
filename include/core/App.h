#pragma once

#include <functional>
#include <iostream>
#include <csignal>
#include "Router.h"
#include "net/Middleware.h"
#include "net/TcpServer.h"
#include "http/Request.h"
#include "http/Response.h"
#include "http/HttpParser.h"

namespace mini_http {

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
        void use(const std::string& prefix, Router& subrouter);
        
        void start(int port);
    private:
        static inline std::atomic<bool> shutdownRequested{false};
        static inline std::condition_variable shutdownCv;
        static inline std::mutex shutdownMutex;
        size_t count;
        Router router;
        MiddlewareChain middlewareChain;
        std::unique_ptr<TcpServer> server;

        bool handleClient(Connection& conn);
        void listen(int port);
        static void signalHandler(int signal);
    };
}