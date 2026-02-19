#pragma once

#include <functional>
#include <atomic>
#include <thread>
#include "ThreadPool.h"
#include "Connection.h"

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "Ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
    #include <poll.h>
#endif

class TcpServer {
public:
    using ConnectionHandler = std::function<bool(Connection&)>;

    TcpServer(int port, size_t threads);
    ~TcpServer();

    void start(ConnectionHandler handler);
    void stop();

private:
    int port;
    ThreadPool pool;

    socket_t serverSocket { INVALID_SOCK };

    std::atomic<bool> running { false };
    std::thread acceptThread;

    #ifndef _WIN32
        int wakeupPipe[2] { INVALID_SOCK, INVALID_SOCK };
    #endif

    void acceptLoop(ConnectionHandler handler);
    void closeSocket(socket_t s);
    void applyReceiveTimeout(socket_t s, int seconds);
};