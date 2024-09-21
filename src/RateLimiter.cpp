#include "../include/RateLimiter.hpp"
#include "../config.hpp"

#include <chrono>

bool RateLimiter::is_rate_limited(const string& client_ip){
    lock_guard<mutex> lock(rate_limit_mutex);
    auto now = chrono::steady_clock::now();

    if(request_map.find(client_ip)!=request_map.end()){
        auto& timestamps = request_map[client_ip];

        while(!timestamps.empty() && chrono::duration_cast<chrono::seconds>(now-timestamps.front()).count()<WINDOW_SIZE){
            timestamps.pop_front();
        }

        if(timestamps.size()>=REQUEST_LIMIT){
            return true;
        }

        timestamps.push_back(now);
    }else{
        request_map[client_ip] = deque<chrono::steady_clock::time_point>{now};
    }
    return false;
}
