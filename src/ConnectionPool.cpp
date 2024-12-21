#include "ConnectionPool.hpp"
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdexcept>
#include <string>

ConnectionPool::ConnectionPool(const string& server_ip, int server_port,int max_pool_size):server_ip(server_ip),server_port(server_port),max_pool_size(max_pool_size){
    initializeConnections();
}

void ConnectionPool::initializeConnections() {
    lock_guard<mutex> lock(pool_mutex);
    for(int i=0;i<max_pool_size;i++){
        int connection_fd = createConnection();
        if(connection_fd >= 0){
            pool.push(connection_fd);
        }else{
            throw runtime_error("Failed to create initial connections for the pool.");
        }
    }
}

ConnectionPool::~ConnectionPool() {
    lock_guard<mutex> lock(pool_mutex);
    while(!pool.empty()){
        close(pool.front());
        pool.pop();
    }
}

ConnectionPool& ConnectionPool::getInstance(const string& ip,int port,int max_conn){
    static ConnectionPool instance(ip, port, max_conn);
    return instance;
}

int ConnectionPool::createConnection() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
        cerr<<"Failed to create socket."<<endl;
        return -1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    if(inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr) <= 0){
        cerr<<"Invalid server IP address."<<endl;
        close(sockfd);
        return -1;
    }

    if(connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        cerr<<"Failed to connect to server."<<endl;
        close(sockfd);
        return -1;
    }

    return sockfd;
}

int ConnectionPool::getConnection() {
    unique_lock<mutex> lock(pool_mutex);
    pool_cv.wait(lock, [this](){ return !pool.empty(); });
    int connection_fd = pool.front();
    pool.pop();
    return connection_fd;
}

void ConnectionPool::releaseConnection(int connection_fd){
    lock_guard<mutex> lock(pool_mutex);
    pool.push(connection_fd);
    pool_cv.notify_one();
}