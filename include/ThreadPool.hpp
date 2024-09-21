#ifndef THREADPOOL_HPP
#define THREADPOOL_HPP

#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>

#include "ClientHandler.hpp"

using namespace std;

class ThreadPool{
private:
    queue<int> task_queue;
    mutex queue_mutex;
    condition_variable condition;
    vector<thread> threads;
    ClientHandler& handler;

public:
    ThreadPool(ClientHandler& handler);
    ~ThreadPool();
    void add_task(int socket_fd);

private:
    void worker_thread();
};

#endif