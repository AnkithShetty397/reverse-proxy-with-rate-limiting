#include <hiredis/read.h>
#include<iostream>
#include<hiredis/hiredis.h>
#include<mutex>

#include "../config.hpp"

using namespace std;

class RedisConnection{
private:
    redisContext* redis_ctx;
    mutex _mutex;
    RedisConnection(){
        redis_ctx = redisConnect("127.0.0.1",6379);
        if(redis_ctx==NULL || redis_ctx->err){
            if(redis_ctx){
                cerr<<"Redis connection error: "<<redis_ctx<<endl;
                redisFree(redis_ctx);
            }else{
                cerr<<"Cannot allocate Redis Context"<<endl;
            }            
            redis_ctx=NULL;
        }
        redisReply* reply =(redisReply*) redisCommand(redis_ctx, "AUTH %s","foobared");
        if(reply==NULL || reply->type==REDIS_REPLY_ERROR){
            cerr<<"Redis authentication failed: "<<(reply? reply->str:"No reply");
            redisFree(redis_ctx);
            redis_ctx=NULL;
        }
        if(reply){
            freeReplyObject(reply);
        }
    }
    ~RedisConnection(){
        if(redis_ctx){
            redisFree(redis_ctx);
        }
    }
public:
    static RedisConnection& getInstance(){
        static RedisConnection instance;
        return instance;
    }

    RedisConnection(const RedisConnection&) = delete;
    RedisConnection& operator =(const RedisConnection&) = delete;

    redisContext* getConnection(){
        return redis_ctx;
    }

    bool setCache(const string& key, const string& value){
        if (!redis_ctx) {
            cerr << "Redis context is not initialized." << endl;
            return false;
        }
        lock_guard<mutex> lock(_mutex);
        redisReply* reply = (redisReply*)redisCommand(redis_ctx,"SET %s %s EX %d",key.c_str(),value.c_str(),REDIS_CACHE_TTL);
        bool success =(reply!=NULL && reply->type ==REDIS_REPLY_STATUS && string(reply->str)=="OK");
        freeReplyObject(reply);
        return success;
    }

    string getCache(const string& key){
        lock_guard<mutex> lock(_mutex);
        redisReply* reply = (redisReply*)redisCommand(redis_ctx,"GET %s",key.c_str());
        string result;
        if(reply!=NULL){
            if(reply->type == REDIS_REPLY_STRING){
                result = reply->str;
            }
            freeReplyObject(reply);
        }
        redisCommand(redis_ctx,"EXPIRE %s %d",key.c_str(),REDIS_CACHE_TTL);
        return result;
    }
};