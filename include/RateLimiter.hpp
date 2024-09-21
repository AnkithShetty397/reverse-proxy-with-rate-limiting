#ifndef CLIENTHANDLER_HPP
#define CLIENTHANDLER_HPP

#include <unordered_map>
#include <deque>
#include <chrono>
#include <string>
#include <mutex>

using namespace std;

class RateLimiter{
private:
    unordered_map<string,deque<chrono::steady_clock::time_point>> request_map;
    mutex rate_limit_mutex;

public:
    bool is_rate_limited(const string& client_ip);
    RateLimiter();
};

#endif