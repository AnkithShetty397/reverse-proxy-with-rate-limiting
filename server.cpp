#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>

#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <vector>

#include <map>
#include <chrono>
#include <deque>

#define PORT 8080
#define REQUEST_LIMIT 5
#define WINDOW_SIZE 10
#define THREAD_POOL_SIZE 4

using namespace std;

// RateLimiter class for managing request limits
class RateLimiter {
private:
    map<string, deque<chrono::steady_clock::time_point>> request_map;
    mutex rate_limit_mutex;

public:
    bool is_rate_limited(const string& client_ip) {
        lock_guard<mutex> lock(rate_limit_mutex);
        auto now = chrono::steady_clock::now();

        if (request_map.find(client_ip) != request_map.end()) {
            auto& timestamps = request_map[client_ip];  //queue dedicated for this client
            
            while (!timestamps.empty() && chrono::duration_cast<chrono::seconds>(now - timestamps.front()).count() > WINDOW_SIZE) {
                timestamps.pop_front(); //Pop the timestamps which are out of window
            }

            if (timestamps.size() >= REQUEST_LIMIT) {
                return true; 
            }
            timestamps.push_back(now);
        } else {
            request_map[client_ip] = deque<chrono::steady_clock::time_point>{now};
        }

        return false;
    }
};

// Handles each client connection
class ClientHandler {
    RateLimiter& rate_limiter;

public:
    ClientHandler(RateLimiter& limiter) : rate_limiter(limiter) {}

    void handle_client(int socket_fd) {
        char buffer[1024] = {0};
        const char* msg = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-length: 19\n\nHello from server";
        const char* rate_limit_msg = "HTTP/1.1 429 Too Many Requests\nContent-Type: text/plain\nContent-length: 30\n\nRate limit exceeded";

        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);

        if (getpeername(socket_fd, (struct sockaddr*)&client_addr, &client_addr_len) == 0) {
            string client_ip = inet_ntoa(client_addr.sin_addr);
            cout << "Client IP address: " << client_ip << endl;

            // Check if the client has exceeded the rate limit
            if (rate_limiter.is_rate_limited(client_ip)) {
                send(socket_fd, rate_limit_msg, strlen(rate_limit_msg), 0);
                cout << "Blocked: " << client_ip << endl;
                close(socket_fd);
                return;
            }
        } else {
            cerr << "Failed to fetch the IP address" << endl;
            close(socket_fd);
            return;
        }

        ssize_t bytes_read = read(socket_fd, buffer, 1024);
        if (bytes_read < 0) {
            cerr << "Error reading from socket" << endl;
            close(socket_fd);
            return;
        }
        cout << "Message received: " << buffer << endl;

        ssize_t bytes_sent = send(socket_fd, msg, strlen(msg), 0);
        if (bytes_sent < 0) {
            cerr << "Error sending response to client" << endl;
        } else {
            cout << "HTTP message sent to client: " << msg << endl;
        }

        close(socket_fd);
    }
};

// Manages the worker threads
class ThreadPool {
    queue<int> task_queue;
    mutex queue_mutex;
    condition_variable condition;
    vector<thread> threads;
    ClientHandler& handler;

public:
    ThreadPool(ClientHandler& handler) : handler(handler) {
        for (int i=0; i<THREAD_POOL_SIZE; i++) {
            threads.emplace_back(&ThreadPool::worker_thread, this);
        }
    }

    ~ThreadPool() {
        for (thread& t : threads) {
            t.join();
        }
    }

    void add_task(int socket_fd) {
        {
            lock_guard<mutex> lock(queue_mutex);
            task_queue.push(socket_fd);
        }
        condition.notify_one();
    }

private:
    void worker_thread() {
        while (true) {
            int socket_fd;
            {
                unique_lock<mutex> lock(queue_mutex);
                condition.wait(lock, [this] { return !task_queue.empty(); });
                socket_fd = task_queue.front();
                task_queue.pop();
            }
            handler.handle_client(socket_fd);
        }
    }
};

// Manages the server socket and accepts connections
class Server {
    int server_fd;
    struct sockaddr_in address;
    socklen_t addrlen;

    ThreadPool& thread_pool;

public:
    Server(ThreadPool& pool) : thread_pool(pool), addrlen(sizeof(address)) {
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd == 0) {
            throw runtime_error("Socket creation failed");
        }

        address.sin_family = AF_INET;
        address.sin_port = htons(PORT);
        address.sin_addr.s_addr = htonl(INADDR_ANY);

        if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
            throw runtime_error("Bind failed");
        }

        if (listen(server_fd, 3) < 0) {
            throw runtime_error("Listen failed");
        }
        cout << "Server is listening on port " << PORT << endl;
    }

    ~Server() {
        close(server_fd);
    }

    void run() {
        while (true) {
            int socket_fd = accept(server_fd, (struct sockaddr*)&address, &addrlen);
            if (socket_fd < 0) {
                cerr << "Accept failed" << endl;
                continue;
            }
            thread_pool.add_task(socket_fd);
        }
    }
};

int main() {
    try {
        RateLimiter rate_limiter;
        ClientHandler handler(rate_limiter);
        ThreadPool thread_pool(handler);
        Server server(thread_pool);

        server.run();
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return -1;
    }

    return 0;
}
