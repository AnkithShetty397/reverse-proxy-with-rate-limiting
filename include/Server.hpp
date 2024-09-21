#ifndef SERVER_HPP
#define SERVER_HPP

#include <netinet/in.h>
#include "ThreadPool.hpp"

class Server{
private:
    int server_fd;
    struct sockaddr_in address;
    socklen_t addrlen;
    ThreadPool& thread_pool;

public:
    Server(ThreadPool& pool);
    ~Server();
    void run();
};

#endif