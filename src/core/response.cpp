#include "../include/core/Response.h"
#include <sstream>

#ifdef _WIN32
    #define CLOSE_SOCKET closesocket
#else
    #include <unistd.h>
    #define CLOSE_SOCKET close
#endif

Response::Response(socket_t socket)
    : socket_(socket),
      status_(HttpStatus::OK),
      sent_(false)
{
}

void Response::setStatus(HttpStatus status) {
    status_ = status;
}

void Response::setHeader(const std::string& key,
                         const std::string& value)
{
    headers_[key] = value;
}

void Response::send(const std::string& body)
{
    if (sent_) return;

    std::string response = buildResponse(body);
    write(response);
    sent_ = true;
}

void Response::json(const std::string& jsonBody)
{
    setHeader("Content-Type", "application/json");
    send(jsonBody);
}

bool Response::isSent() const {
    return sent_;
}

std::string Response::buildResponse(const std::string& body) const
{
    std::ostringstream ss;

    int statusCode = static_cast<int>(status_);

    ss << "HTTP/1.1 " << statusCode << " \r\n";

    for (const auto& [key, value] : headers_) {
        ss << key << ": " << value << "\r\n";
    }

    ss << "Content-Length: " << body.size() << "\r\n";
    ss << "\r\n";
    ss << body;

    return ss.str();
}

void Response::write(const std::string& data)
{
#ifdef _WIN32
    ::send(socket_, data.c_str(),
           static_cast<int>(data.size()), 0);
#else
    ::send(socket_, data.c_str(),
           data.size(), 0);
#endif
}