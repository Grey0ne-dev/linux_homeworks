#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <cerrno>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>




int main() {
    const char* HOST = "httpforever.com";
    const char* PORT = "80";
    const std::string FILENAME = "httpforever.html";

    struct addrinfo hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo* res = nullptr;
    int rc = getaddrinfo(HOST, PORT, &hints, &res);
    if (rc != 0) {
        std::cerr << "getaddrinfo: " << gai_strerror(rc) << std::endl;
        return 1;
    }

    int sock = -1;
    struct addrinfo* rp;
    for (rp = res; rp != nullptr; rp = rp->ai_next) {
        sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sock == -1) continue;
        if (connect(sock, rp->ai_addr, rp->ai_addrlen) == 0) break;
        close(sock);
        sock = -1;
    }
    freeaddrinfo(res);

    if (sock == -1) {
        std::cerr << "Could not connect to " << HOST << std::endl;
        return 2;
    }

    std::string request = "GET / HTTP/1.1\r\n";
    request += "Host: "; request += HOST; request += "\r\n";
    request += "User-Agent: cpp-socket-client\r\n";
    request += "Connection: close\r\n\r\n";

    ssize_t sent = 0;
    const char* buf = request.c_str();
    size_t to_send = request.size();
    while (to_send > 0) {
        ssize_t n = send(sock, buf + sent, to_send, 0);
        if (n <= 0) {
            std::cerr << "send error: " << strerror(errno) << std::endl;
            close(sock);
            return 3;
        }
        sent += n;
        to_send -= n;
    }

    int fd = open(FILENAME.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        std::cerr << "Failed to open file for writing: " << FILENAME << " - " << strerror(errno) << std::endl;
        close(sock);
        return 4;
    }

    std::vector<char> recvbuf(4096);
    std::string accumulated;
    while (true) {
        ssize_t n = recv(sock, recvbuf.data(), (int)recvbuf.size(), 0);
        if (n < 0) {
            std::cerr << "recv error: " << strerror(errno) << std::endl;
            close(fd);
            close(sock);
            return 5;
        }
        if (n == 0) break;
        ssize_t wrote = 0;
        const char* wp = recvbuf.data();
        ssize_t to_write = n;
        while (to_write > 0) {
            ssize_t w = write(fd, wp + wrote, to_write);
            if (w <= 0) {
                std::cerr << "write error: " << strerror(errno) << std::endl;
                close(fd);
                close(sock);
                return 7;
            }
            wrote += w;
            to_write -= w;
        }
        accumulated.append(recvbuf.data(), recvbuf.data() + n);
    }

    close(fd);
    close(sock);

    if (accumulated.find("HTTP/1.1 200 OK") != std::string::npos) {
        std::cout << "Received HTTP/1.1 200 OK; saved to " << FILENAME << std::endl;
        return 0;
    } else {
        std::cerr << "Did not find expected status line 'HTTP/1.1 200 OK' in response" << std::endl;
        return 6;
    }
}
