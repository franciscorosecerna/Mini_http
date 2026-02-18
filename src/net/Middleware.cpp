#include "../include/net/Middleware.h"
#include <memory>

void MiddlewareChain::use(Middleware middleware) {
    middlewares.push_back(std::move(middleware));
}

void MiddlewareChain::execute(
    Request& req,
    Response& res,
    std::function<void()> finalHandler)
{
    auto index = std::make_shared<size_t>(0);
    auto self = this;

    auto next = std::make_shared<std::function<void()>>();

    *next = [&, index, self, next, finalHandler]() {
        if (*index >= self->middlewares.size()) {
            finalHandler();
            return;
        }

        Middleware& mw = self->middlewares[(*index)++];
        mw(req, res, *next);
    };

    (*next)();
}