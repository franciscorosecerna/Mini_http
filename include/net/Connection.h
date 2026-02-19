#pragma once
#include <HttpParser.h>
#include <functional>
#include <atomic>
#include <thread>
#include "ThreadPool.h"

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "Ws2_32.lib")

    using socket_t = SOCKET;
    static constexpr socket_t INVALID_SOCK = INVALID_SOCKET;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
    #include <poll.h>
    #include <sys/types.h>

    using socket_t = int;
    static constexpr socket_t INVALID_SOCK = -1;
#endif


class Connection {
public:
    std::string readBuffer;
    explicit Connection(socket_t fd) : fd(fd) {}

    ~Connection() { close(); }

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;
    Connection(Connection&& o) noexcept : fd(o.fd) { o.fd = INVALID_SOCK; }

    socket_t raw() const { return fd; }

    ssize_t read(void* buf, size_t len) const {
        #ifdef _WIN32
            return ::recv(fd, static_cast<char*>(buf), static_cast<int>(len), 0);
        #else
            return ::recv(fd, buf, len, 0);
        #endif
    }

    ssize_t write(const void* buf, size_t len) const {
        #ifdef _WIN32
            return ::send(fd, static_cast<const char*>(buf), static_cast<int>(len), 0);
        #else
            return ::send(fd, buf, len, 0);
        #endif
    }

    void close() {
        if (fd == INVALID_SOCK) return;
        #ifdef _WIN32
            closesocket(fd);
        #else
            ::close(fd);
        #endif
        fd = INVALID_SOCK;
    }

private:
    socket_t fd;
};