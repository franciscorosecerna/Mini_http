#include "../include/core/App.h"

void App::get(const std::string& path, Handler handler) {
    router.add(HttpMethod::GET, path, handler);
}

void App::post(const std::string& path, Handler handler) {
    router.add(HttpMethod::POST, path, handler);
}

void App::put(const std::string& path, Handler handler) {
    router.add(HttpMethod::PUT, path, handler);
}

void App::del(const std::string& path, Handler handler) {
    router.add(HttpMethod::DELETE_, path, handler);
}

void App::patch(const std::string& path, Handler handler) {
    router.add(HttpMethod::PATCH, path, handler);
}

void App::options(const std::string& path, Handler handler) {
    router.add(HttpMethod::OPTIONS, path, handler);
}

void App::head(const std::string& path, Handler handler) {
    router.add(HttpMethod::HEAD, path, handler);
}

void App::use(Middleware middleware) {
    middlewareChain.use(middleware);
}

void App::use(const std::string& prefix, Router& subrouter) {
    std::string base = prefix;
    if (!base.empty() && base.back() == '/') {
        base.pop_back();
    }

    for (const auto& [method, path, handler] : subrouter.getMountableRoutes()) {
        std::string fullPath = base;
        if (!path.empty() && path != "/") {
            fullPath += path;
        }
        router.add(method, fullPath, handler);
    }
}

App::App(size_t threads) : count(threads) {}

void App::listen(int port) {
    server = std::make_unique<TcpServer>(port, count);
    server->start([this](Connection& conn) {
        return handleClient(conn);
    });
}

bool App::handleClient(Connection& conn) {
    try {
        Request req;
        try {
            req = parseRequest(conn);
        } catch (const std::runtime_error& e) {
            return false;
        }

        Response res(conn.raw());

        middlewareChain.execute(req, res, [&]() {
            if (!router.dispatch(req, res)) {
                res.setStatus(HttpStatus::NOT_FOUND);
                res.send("Not Found");
            }
        });

        return req.keepAlive();

    } catch (const std::exception& e) {
        try {
            Response res(conn.raw());
            res.setStatus(HttpStatus::INTERNAL_SERVER_ERROR);
            res.send("Internal Server Error");
        } catch (...) {
        }
        return false;
    }
}