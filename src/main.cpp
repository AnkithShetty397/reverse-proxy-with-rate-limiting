#include "../include/RateLimiter.hpp"
#include "../include/ClientHandler.hpp"
#include "../include/ThreadPool.hpp"
#include "../include/Server.hpp"
#include <iostream>

int main(){
    try{
        RateLimiter rate_limiter;
        ClientHandler handler(rate_limiter);
        ThreadPool thread_pool(handler);
        Server server(thread_pool);

        server.run();
    }catch(const std::exception& e){
        std::cerr<<"Error: "<<e.what()<<std::endl;
        return -1;
    }
    return 0;
}