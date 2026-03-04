#pragma once

#include "nlohmann/json.hpp"
#include <string>
#include <unordered_map>

#ifdef _WIN32
    #include <winsock2.h>
    using socket_t = SOCKET;
#else
    using socket_t = int;
#endif

#include "httpstatus.h"

namespace mini_http {
class Response {
    public:
        explicit Response(socket_t socket);

        void setStatus(HttpStatus status);
        void setHeader(const std::string& key,
                    const std::string& value);

        void send(const std::string& body);
        void json(const nlohmann::json& data);

        void redirect(const std::string& location,
                HttpStatus status = HttpStatus::FOUND);

        bool isSent() const;

        // 2xx
        void ok(const nlohmann::json& data);
        void created(const nlohmann::json& data);
        void noContent();

        // 3xx
        void movedPermanently(const std::string& location);
        void found(const std::string& location);
        void seeOther(const std::string& location);
        void temporaryRedirect(const std::string& location);
        void permanentRedirect(const std::string& location);

        // 4xx
        void badRequest(const std::string& message = "Bad Request");
        void unauthorized(const std::string& message = "Unauthorized");
        void forbidden(const std::string& message = "Forbidden");
        void notFound(const std::string& message = "Not Found");
        void methodNotAllowed(const std::string& allow, const std::string& message = "Method Not Allowed");

        // 5xx
        void internalServerError(const std::string& message = "Internal Server Error");
    private:
        socket_t socket_;
        HttpStatus status_;
        std::unordered_map<std::string, std::string> headers_;
        bool sent_;

        std::string buildResponse(const std::string& body) const;
        void write(const std::string& data);
        void sendError(HttpStatus status, const std::string& message);
    };
}