#include "http/HttpParser.h"
#include "net/Connection.h"
#include <sstream>
#include <algorithm>
#include <stdexcept>

namespace mini_http {
    static HttpMethod parseMethod(const std::string& methodStr) {
        if (methodStr == "GET") return HttpMethod::GET;
        if (methodStr == "POST") return HttpMethod::POST;
        if (methodStr == "PUT") return HttpMethod::PUT;
        if (methodStr == "DELETE") return HttpMethod::DELETE_;
        if (methodStr == "PATCH") return HttpMethod::PATCH;
        if (methodStr == "OPTIONS") return HttpMethod::OPTIONS;
        if (methodStr == "HEAD") return HttpMethod::HEAD;

        throw std::runtime_error("Unsupported HTTP method");
    }

    Request parseRequest(Connection& conn) {
        std::string& raw = conn.readBuffer;
        char buffer[4096];

        while (raw.find("\r\n\r\n") == std::string::npos) {
            ssize_t bytes = conn.read(buffer, sizeof(buffer));

            if (bytes == 0) {
                if (raw.empty())
                    throw std::runtime_error("Connection closed");
                break;
            }

            if (bytes < 0)
                throw std::runtime_error("Socket read error");

            raw.append(buffer, bytes);

            if (raw.size() > 8192)
                throw std::runtime_error("Header too large");
        }

        size_t headerEnd = raw.find("\r\n\r\n");
        std::string headerPart = raw.substr(0, headerEnd);

        std::istringstream stream(headerPart);

        std::string methodStr, path, version;
        stream >> methodStr >> path >> version;

        if (methodStr.empty() || path.empty() || version.empty())
            throw std::runtime_error("Malformed request line");

        Request req;
        req.method = parseMethod(methodStr);
        req.version = version;

        size_t qmark = path.find('?');
        if (qmark != std::string::npos) {
            req.query = path.substr(qmark + 1);
            req.path  = path.substr(0, qmark);
        } else {
            req.path = path;
        }

        std::string line;
        std::getline(stream, line);

        while (std::getline(stream, line)) {
            if (!line.empty() && line.back() == '\r')
                line.pop_back();

            if (line.empty())
                continue;

            size_t colon = line.find(":");
            if (colon == std::string::npos)
                continue;

            std::string key = line.substr(0, colon);
            std::string value = line.substr(colon + 1);

            if (!value.empty() && value[0] == ' ')
                value.erase(0, 1);

            std::transform(key.begin(), key.end(), key.begin(), ::tolower);

            req.headers[key] = value;
        }

        size_t contentLength = 0;
        auto it = req.headers.find("content-length");
        if (it != req.headers.end()) {
            contentLength = std::stoul(it->second);
        }

        size_t totalRequestSize = headerEnd + 4 + contentLength;

        while (raw.size() < totalRequestSize) {
            ssize_t bytes = conn.read(buffer, sizeof(buffer));

            if (bytes <= 0)
                throw std::runtime_error("Body truncated");

            raw.append(buffer, bytes);
        }

        if (contentLength > 0) {
            req.body = raw.substr(headerEnd + 4, contentLength);
        }

        raw.erase(0, totalRequestSize);

        return req;
    }
}