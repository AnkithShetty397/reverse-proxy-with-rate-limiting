#ifndef THREADPOOL_HPP
#define THREADPOOL_HPP

#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>

#include "SSLClientHandler.hpp"

using namespace std;

class ThreadPool{
private:
    queue<int> task_queue;
    mutex queue_mutex;
    condition_variable condition;
    vector<thread> threads;
    SSLClientHandler& handler;

public:
    ThreadPool(SSLClientHandler& handler);
    ~ThreadPool();
    void add_task(int socket_fd);
    size_t get_queue_size();

private:
    void worker_thread();
};

#endif