#include "../include/net/TcpServer.h"
#include <iostream>
#include <stdexcept>
#include <cerrno>

TcpServer::TcpServer(int port, size_t threads)
    : port(port), pool(threads) {}

TcpServer::~TcpServer() {
    stop();
    if (acceptThread.joinable())
        acceptThread.join();
}

void TcpServer::start(ConnectionHandler handler) {
    if (running.exchange(true)) return;

    #ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
            throw std::runtime_error("WSAStartup failed");
    #else
        if (pipe(wakeupPipe) != 0)
            throw std::runtime_error("Failed to create wakeup pipe");
    #endif

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCK) {
        #ifndef _WIN32
            ::close(wakeupPipe[0]);
            ::close(wakeupPipe[1]);
        #endif
        throw std::runtime_error("Failed to create socket");
    }

    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR,
               reinterpret_cast<const char*>(&opt), sizeof(opt));
    #ifndef _WIN32
        setsockopt(serverSocket, SOL_SOCKET, SO_REUSEPORT,
                   reinterpret_cast<const char*>(&opt), sizeof(opt));
    #endif

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(serverSocket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        closeSocket(serverSocket);
        throw std::runtime_error("Bind failed");
    }

    if (listen(serverSocket, 128) < 0) {
        closeSocket(serverSocket);
        throw std::runtime_error("Listen failed");
    }

    std::cout << "Server running on port " << port << "\n";

    acceptThread = std::thread(&TcpServer::acceptLoop, this, std::move(handler));
    acceptThread.join();
}

void TcpServer::stop() {
    if (!running.exchange(false)) return;

    #ifndef _WIN32
        if (wakeupPipe[1] != INVALID_SOCK) {
            char byte = 1;
            (void)::write(wakeupPipe[1], &byte, 1);
        }
    #endif

    if (serverSocket != INVALID_SOCK) {
        #ifdef _WIN32
            shutdown(serverSocket, SD_BOTH);
        #else
            shutdown(serverSocket, SHUT_RDWR);
        #endif
        closeSocket(serverSocket);
        serverSocket = INVALID_SOCK;
    }

    pool.shutdown();

    #ifdef _WIN32
        WSACleanup();
    #endif
}

void TcpServer::acceptLoop(ConnectionHandler handler) {
    while (running.load()) {

        #ifndef _WIN32
            struct pollfd fds[2];
            fds[0].fd = serverSocket;
            fds[0].events = POLLIN;
            fds[1].fd = wakeupPipe[0];
            fds[1].events = POLLIN;

            int ready = poll(fds, 2, -1);
            if (ready < 0) {
                if (errno == EINTR) continue;
                std::cerr << "poll() failed: " << strerror(errno) << "\n";
                break;
            }

            if (fds[1].revents & POLLIN) break;

            if (!(fds[0].revents & POLLIN)) continue;
        #endif

        socket_t clientSocket = accept(serverSocket, nullptr, nullptr);

        if (clientSocket == INVALID_SOCK) {
            if (!running.load()) break;
            #ifndef _WIN32
                if (errno == EINTR) continue;
            #endif
            std::cerr << "accept() failed\n";
            continue;
        }

        applyReceiveTimeout(clientSocket, 10);

        try {
            pool.enqueue([handler, conn = std::make_shared<Connection>(clientSocket)]() {
            try {
                bool keepAlive = true;
                while (keepAlive)
                    keepAlive = handler(*conn);
            }
            catch (const std::exception& e) {
                    std::cerr << "Handler exception: " << e.what() << "\n";
            }
            catch (...) {
                std::cerr << "Handler exception\n";
            }
            });
        }
        catch (const std::exception& e) {
            std::cerr << "Failed to enqueue task: " << e.what() << "\n";
            closeSocket(clientSocket);
        }
    }

    #ifndef _WIN32
        if (wakeupPipe[0] != INVALID_SOCK) { ::close(wakeupPipe[0]); wakeupPipe[0] = INVALID_SOCK; }
        if (wakeupPipe[1] != INVALID_SOCK) { ::close(wakeupPipe[1]); wakeupPipe[1] = INVALID_SOCK; }
    #endif
}

void TcpServer::closeSocket(socket_t s) {
    if (s == INVALID_SOCK) return;
    #ifdef _WIN32
        closesocket(s);
    #else
        ::close(s);
    #endif
}

void TcpServer::applyReceiveTimeout(socket_t s, int seconds) {
    #ifdef _WIN32
        DWORD timeout = static_cast<DWORD>(seconds) * 1000;
        if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO,
                       reinterpret_cast<const char*>(&timeout), sizeof(timeout)) != 0)
            std::cerr << "Warning: failed to set SO_RCVTIMEO\n";
    #else
        struct timeval tv { seconds, 0 };
        if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) != 0)
            std::cerr << "Warning: failed to set SO_RCVTIMEO\n";
    #endif
}