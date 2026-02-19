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

App::App(size_t threads) : count(threads) {}

void App::listen(int port) {
    server = std::make_unique<TcpServer>(port, count);
    server->start([this](socket_t clientSocket) {
        handleClient(clientSocket);
    });
}

void App::handleClient(socket_t clientSocket) {
    try {
        Request req = parseRequest(clientSocket);
        Response res(clientSocket);

        middlewareChain.execute(req, res, [&]() {
            if (!router.dispatch(req, res)) {
                res.setStatus(HttpStatus::NOT_FOUND);
                res.send("Not Found");
            }
        });
    }
    catch (const std::exception& e) {
        Response res(clientSocket);
        res.setStatus(HttpStatus::INTERNAL_SERVER_ERROR);
        res.send("Internal Server Error");
    }
}
