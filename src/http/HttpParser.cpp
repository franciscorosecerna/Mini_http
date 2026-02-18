#include "../include/http/HttpParser.h"
#include <sstream>
#include <iostream>

Request parseRequest(socket_t clientSocket) {
    char buffer[4096] = {0};
#ifdef _WIN32
    int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
#else
    int bytesRead = read(clientSocket, buffer, sizeof(buffer));
#endif

    if (bytesRead <= 0) throw std::runtime_error("Empty request");

    std::istringstream stream(buffer);
    std::string methodStr, path, version;
    stream >> methodStr >> path >> version;

    HttpMethod method = HttpMethod::OPTIONS;
    if (methodStr == "GET") method = HttpMethod::GET;
    else if (methodStr == "POST") method = HttpMethod::POST;
    else if (methodStr == "PUT") method = HttpMethod::PUT;
    else if (methodStr == "DELETE") method = HttpMethod::DELETE_;
    else if (methodStr == "PATCH") method = HttpMethod::PATCH;
    else if (methodStr == "OPTIONS") method = HttpMethod::OPTIONS;
    else if (methodStr == "HEAD") method = HttpMethod::HEAD;

    Request req;
    req.method = method;
    req.path = path;

    return req;
}