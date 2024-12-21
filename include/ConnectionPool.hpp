#ifndef CONNECTION_POOL_HPP
#define CONNECTION_POOL_HPP

#include <queue>
#include <mutex>
#include <condition_variable>
#include <netinet/in.h>
#include <unistd.h>
#include <string>

#include "../config.hpp"

using namespace std;

class ConnectionPool {
private:
    string server_ip;
    int server_port;

    int max_pool_size;
    queue<int> pool;
    mutex pool_mutex;
    condition_variable pool_cv;

    ConnectionPool(const string& server_ip, int server_port, int max_pool_size);
    
    void initializeConnections();
    int createConnection();

    ConnectionPool(const ConnectionPool&) = delete;
    ConnectionPool& operator=(const ConnectionPool&) = delete;

public:
    static ConnectionPool& getInstance(const string& ip = string(INTERNAL_SERVER_IP), int port = INTERNAL_SERVER_PORT, int max_conn = MAX_POOL_SIZE);
    int getConnection();
    void releaseConnection(int connection_fd);
    ~ConnectionPool();

};

#endif