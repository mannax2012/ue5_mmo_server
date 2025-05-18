#include "RedisClient.h"
#include <hiredis/hiredis.h>
#include <map>
#include <vector>
#include <string>

RedisClient::RedisClient() : ctx(nullptr) {}
RedisClient::~RedisClient() { if (ctx) redisFree(ctx); }

bool RedisClient::connect(const std::string& host, int port, const std::string& user, const std::string& password) {
    if (ctx) redisFree(ctx);
    ctx = redisConnect(host.c_str(), port);
    if (!ctx || ctx->err) {
        if (ctx) { redisFree(ctx); ctx = nullptr; }
        return false;
    }
    if (!password.empty()) {
        redisReply* reply = (redisReply*)redisCommand(ctx, "AUTH %s %s", user.c_str(), password.c_str());
        if (!reply || reply->type == REDIS_REPLY_ERROR) {
            if (reply) freeReplyObject(reply);
            redisFree(ctx); ctx = nullptr;
            return false;
        }
        freeReplyObject(reply);
    }
    return true;
}

bool RedisClient::set(const std::string& key, const std::string& value) {
    if (!ctx) return false;
    redisReply* reply = (redisReply*)redisCommand(ctx, "SET %s %s", key.c_str(), value.c_str());
    bool ok = reply && (reply->type == REDIS_REPLY_STATUS || reply->type == REDIS_REPLY_STRING);
    if (reply) freeReplyObject(reply);
    return ok;
}

bool RedisClient::get(const std::string& key, std::string& value) {
    if (!ctx) return false;
    redisReply* reply = (redisReply*)redisCommand(ctx, "GET %s", key.c_str());
    if (reply && reply->type == REDIS_REPLY_STRING) {
        value = reply->str;
        freeReplyObject(reply);
        return true;
    }
    if (reply) freeReplyObject(reply);
    return false;
}

bool RedisClient::del(const std::string& key) {
    if (!ctx) return false;
    redisReply* reply = (redisReply*)redisCommand(ctx, "DEL %s", key.c_str());
    bool ok = reply && reply->type == REDIS_REPLY_INTEGER && reply->integer > 0;
    if (reply) freeReplyObject(reply);
    return ok;
}

bool RedisClient::exists(const std::string& key) {
    if (!ctx) return false;
    redisReply* reply = (redisReply*)redisCommand(ctx, "EXISTS %s", key.c_str());
    bool ok = reply && reply->type == REDIS_REPLY_INTEGER && reply->integer > 0;
    if (reply) freeReplyObject(reply);
    return ok;
}

bool RedisClient::expire(const std::string& key, int seconds) {
    if (!ctx) return false;
    redisReply* reply = (redisReply*)redisCommand(ctx, "EXPIRE %s %d", key.c_str(), seconds);
    bool ok = reply && reply->type == REDIS_REPLY_INTEGER && reply->integer == 1;
    if (reply) freeReplyObject(reply);
    return ok;
}

bool RedisClient::hset(const std::string& key, const std::vector<std::pair<std::string, std::string>>& fields) {
    if (!ctx) return false;
    std::vector<std::string> argStorage; // Keep all strings alive
    argStorage.push_back("HSET");
    argStorage.push_back(key);
    for (const auto& kv : fields) {
        argStorage.push_back(kv.first);
        argStorage.push_back(kv.second);
    }
    std::vector<const char*> argv;
    std::vector<size_t> argvlen;
    for (const auto& s : argStorage) {
        argv.push_back(s.c_str());
        argvlen.push_back(s.size());
    }
    redisReply* reply = (redisReply*)redisCommandArgv(ctx, argv.size(), argv.data(), argvlen.data());
    bool ok = reply && (reply->type == REDIS_REPLY_INTEGER || reply->type == REDIS_REPLY_STATUS);
    if (reply) freeReplyObject(reply);
    return ok;
}

bool RedisClient::keys(const std::string& pattern, std::vector<std::string>& outKeys) {
    if (!ctx) return false;
    redisReply* reply = (redisReply*)redisCommand(ctx, "KEYS %s", pattern.c_str());
    if (!reply || reply->type != REDIS_REPLY_ARRAY) {
        if (reply) freeReplyObject(reply);
        return false;
    }
    outKeys.clear();
    for (size_t i = 0; i < reply->elements; ++i) {
        outKeys.push_back(reply->element[i]->str);
    }
    freeReplyObject(reply);
    return true;
}

bool RedisClient::hgetall(const std::string& key, std::map<std::string, std::string>& outFields) {
    if (!ctx) return false;
    redisReply* reply = (redisReply*)redisCommand(ctx, "HGETALL %s", key.c_str());
    if (!reply || reply->type != REDIS_REPLY_ARRAY || reply->elements % 2 != 0) {
        if (reply) freeReplyObject(reply);
        return false;
    }
    outFields.clear();
    for (size_t i = 0; i < reply->elements; i += 2) {
        std::string field = reply->element[i]->str;
        std::string value = reply->element[i+1]->str;
        outFields[field] = value;
    }
    freeReplyObject(reply);
    return true;
}
