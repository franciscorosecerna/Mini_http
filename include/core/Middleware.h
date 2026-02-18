#pragma once
#include <functional>
#include <vector>

struct Request;
class Response;

using Next = std::function<void()>;
using Middleware = std::function<void(Request&, Response&, Next)>;

class MiddlewareChain {
public:
    void use(Middleware middleware);

    void execute(
        Request& req,
        Response& res,
        std::function<void()> finalHandler
    );

private:
    std::vector<Middleware> middlewares;
};