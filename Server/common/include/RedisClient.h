#pragma once
#include <string>
#include <vector>
#include <utility> // for std::pair
#include <map>     // for std::map

struct redisContext;

class RedisClient {
public:
    RedisClient();
    ~RedisClient();
    bool connect(const std::string& host, int port, const std::string& user, const std::string& password);
    bool set(const std::string& key, const std::string& value);
    bool get(const std::string& key, std::string& value);
    bool del(const std::string& key);
    bool exists(const std::string& key);
    bool expire(const std::string& key, int seconds);
    bool hset(const std::string& key, const std::vector<std::pair<std::string, std::string>>& fields);
    bool keys(const std::string& pattern, std::vector<std::string>& outKeys);
    bool hgetall(const std::string& key, std::map<std::string, std::string>& outFields);
private:
    redisContext* ctx;
};
