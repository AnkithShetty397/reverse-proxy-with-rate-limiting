#include "../include/RateLimiter.hpp"
#include "../include/SSLClientHandler.hpp"
#include "../include/SSLHandler.hpp"
#include "../include/ThreadPool.hpp"
#include "../include/Server.hpp"

#include <iostream>

int main(){
    SSLHandler ssl_handler("../cert/cert.pem", "../cert/key.pem");
    if(!ssl_handler.init_ssl_context()){
        std::cerr<<"Failed to initialize SSL context."<<std::endl;
        return -1;
    }
    try{
        RateLimiter rate_limiter;
        SSLClientHandler handler(rate_limiter,ssl_handler);
        ThreadPool thread_pool(handler);
        Server server(thread_pool);

        server.run();
    }catch(const std::exception& e){
        std::cerr<<"Error: "<<e.what()<<std::endl;
        return -1;
    }
    return 0;
}