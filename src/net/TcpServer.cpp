#include "../include/net/TcpServer.h"
#include <iostream>
#include <stdexcept>

TcpServer::TcpServer(int port, size_t threads)
    : port(port), pool(threads) {}

TcpServer::~TcpServer() {
    stop();
}

void TcpServer::closeSocket(socket_t s) {
    #ifdef _WIN32
        closesocket(s);
    #else
        close(s);
    #endif
}

void TcpServer::start(ConnectionHandler handler) {

    #ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
            throw std::runtime_error("WSAStartup failed");
        }
    #endif

        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    #ifdef _WIN32
        if (serverSocket == INVALID_SOCKET)
    #else
        if (serverSocket < 0)
    #endif
        {
            throw std::runtime_error("Failed to create socket");
        }

    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR,
               reinterpret_cast<char*>(&opt), sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(serverSocket, (sockaddr*)&addr, sizeof(addr)) < 0) {
        closeSocket(serverSocket);
        throw std::runtime_error("Bind failed");
    }

    if (listen(serverSocket, 128) < 0) {
        closeSocket(serverSocket);
        throw std::runtime_error("Listen failed");
    }

    running.store(true);

    std::cout << "Server running on port " << port << "\n";

    while (running.load()) {

        socket_t clientSocket = accept(serverSocket, nullptr, nullptr);

        #ifdef _WIN32
            bool acceptError = (clientSocket == INVALID_SOCKET);
        #else
            bool acceptError = (clientSocket < 0);
        #endif

        if (acceptError) {

            if (!running.load()) {
                break;
            }

        #ifndef _WIN32
            if (errno == EINTR)
                continue;
        #endif
            std::cerr << "Accept failed\n";
            continue;
        }

        try {
            pool.enqueue([this, handler, clientSocket]() {
                try {
                    handler(clientSocket);
                }
                catch (const std::exception& e) {
                    std::cerr << "Handler error: " << e.what() << "\n";
                }

                closeSocket(clientSocket);
            });
        }
        catch (...) {
            closeSocket(clientSocket);
        }
    }

    pool.shutdown();

    #ifdef _WIN32
        WSACleanup();
    #endif
}

void TcpServer::stop() {
    if (!running.exchange(false))
        return;

    if (serverSocket != -1) {

    #ifdef _WIN32
        shutdown(serverSocket, SD_BOTH);
    #else
        shutdown(serverSocket, SHUT_RDWR);
    #endif

        closeSocket(serverSocket);
        serverSocket = -1;
    }
}