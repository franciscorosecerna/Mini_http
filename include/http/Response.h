#pragma once

#include <string>
#include <unordered_map>

#ifdef _WIN32
    #include <winsock2.h>
    using socket_t = SOCKET;
#else
    using socket_t = int;
#endif

#include "httpstatus.h"

class Response {
public:
    explicit Response(socket_t socket);

    void setStatus(HttpStatus status);
    void setHeader(const std::string& key,
                   const std::string& value);

    void send(const std::string& body);
    void json(const std::string& jsonBody);

    bool isSent() const;

private:
    socket_t socket_;
    HttpStatus status_;
    std::unordered_map<std::string, std::string> headers_;
    bool sent_;

    std::string buildResponse(const std::string& body) const;
    void write(const std::string& data);
};