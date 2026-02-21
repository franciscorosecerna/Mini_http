#include "http/Response.h"
#include <sstream>

#ifdef _WIN32
    #define CLOSE_SOCKET closesocket
#else
    #include <unistd.h>
    #define CLOSE_SOCKET close
#endif

namespace mini_http {
    static const char* reasonPhrase(HttpStatus status) {
        switch (status) {
            case HttpStatus::OK: return "OK";
            case HttpStatus::CREATED: return "Created";
            case HttpStatus::NO_CONTENT: return "No Content";
            case HttpStatus::MOVED_PERMANENTLY: return "Moved Permanently";
            case HttpStatus::FOUND: return "Found";
            case HttpStatus::SEE_OTHER: return "See Other";
            case HttpStatus::TEMPORARY_REDIRECT: return "Temporary Redirect";
            case HttpStatus::PERMANENT_REDIRECT: return "Permanent Redirect";
            case HttpStatus::BAD_REQUEST: return "Bad Request";
            case HttpStatus::UNAUTHORIZED: return "Unauthorized";
            case HttpStatus::FORBIDDEN: return "Forbidden";
            case HttpStatus::NOT_FOUND: return "Not Found";
            case HttpStatus::METHOD_NOT_ALLOWED: return "Method Not Allowed";
            case HttpStatus::INTERNAL_SERVER_ERROR: return "Internal Server Error";
            default: return "";
        }
    }

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

        if (headers_.find("Content-Type") == headers_.end())
        headers_["Content-Type"] = "text/plain";

        if (headers_.find("Connection") == headers_.end())
        headers_["Connection"] = "close";


        std::string response = buildResponse(body);
        write(response);
        sent_ = true;
    }

    void Response::json(const std::string& jsonBody)
    {
        setHeader("Content-Type", "application/json");
        send(jsonBody);
    }

    void Response::redirect(const std::string& location,
                            HttpStatus status)
    {
        if (sent_) return;

        setStatus(status);
        setHeader("Location", location);

        setHeader("Content-Type", "text/html");
        send("");
    }

    bool Response::isSent() const {
        return sent_;
    }

    std::string Response::buildResponse(const std::string& body) const
    {
        std::ostringstream ss;

        int statusCode = static_cast<int>(status_);

        ss << "HTTP/1.1 "
        << statusCode << " "
        << reasonPhrase(status_)
        << "\r\n";

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
        size_t totalSent = 0;
        size_t totalSize = data.size();

        while (totalSent < totalSize) {

    #ifdef _WIN32
            int sent = ::send(socket_,
                            data.c_str() + totalSent,
                            static_cast<int>(totalSize - totalSent),
                            0);
    #else
            ssize_t sent = ::send(socket_,
                                data.c_str() + totalSent,
                                totalSize - totalSent,
                                0);
    #endif

            if (sent <= 0) {
    #ifndef _WIN32
        if (errno == EINTR) continue;
    #endif
            throw std::runtime_error("Socket send failed");
            }
            totalSent += sent;
        }
    }
}