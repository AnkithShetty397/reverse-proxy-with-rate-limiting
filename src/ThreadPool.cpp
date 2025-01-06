#include "../include/ThreadPool.hpp"
#include "../config.hpp"

ThreadPool::ThreadPool(SSLClientHandler& handler): handler(handler){
    for(int i=0;i<THREAD_POOL_SIZE;i++){
        threads.emplace_back(&ThreadPool::worker_thread, this);
    }
}

ThreadPool::~ThreadPool(){
    for(thread& t: threads){
        t.join();
    }
}

void ThreadPool::add_task(int socket_fd){
    {
        lock_guard<mutex> lock(queue_mutex);
        task_queue.push(socket_fd);
    }
    condition.notify_one();
}

void ThreadPool::worker_thread(){
    while(true){
        int socket_fd;
        {
            unique_lock<mutex> lock(queue_mutex);
            condition.wait(lock,[this]{ return !task_queue.empty(); });
            socket_fd = task_queue.front();
            task_queue.pop();
        }
        handler.handle_client(socket_fd);
    }
}

size_t ThreadPool::get_queue_size(){
    unique_lock<mutex> lock(queue_mutex);
    return task_queue.size();
}