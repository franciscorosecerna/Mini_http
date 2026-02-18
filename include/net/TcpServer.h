#pragma once

#include <functional>
#include <atomic>
#include "ThreadPool.h"

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "Ws2_32.lib")
    using socket_t = SOCKET;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
    using socket_t = int;
#endif

class TcpServer {
public:
    using ConnectionHandler = std::function<void(socket_t)>;

    TcpServer(int port, size_t threads);
    ~TcpServer();

    void start(ConnectionHandler handler);
    void stop();

private:
    int port;
    ThreadPool pool;
    socket_t serverSocket{};
    std::atomic<bool> running{false};

    void closeSocket(socket_t s);
};