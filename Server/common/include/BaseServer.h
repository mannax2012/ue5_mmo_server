#pragma once
#include "Config.h"
#include "RedisClient.h"
#include "SocketServer.h"
#include "MySQLClient.h" // Include MySQLClient header
#include "Packets.h"
#include <string>
#include <atomic>
#include <thread>
#include <functional>
#include <csignal>
#include <iostream>
#include <unordered_map>
#include <random>
#include <chrono>
#include <memory> // For std::shared_ptr

class Player; // Forward declaration of Player

struct SessionInfo {
    bool connected = true;
    std::chrono::steady_clock::time_point lastHeartbeat;
    int32_t playerId = -1;
    int32_t charId = -1;
    std::string username;
    std::string sessionKey;
    std::shared_ptr<Player> playerEntity; // Pointer to the Player entity for this session
};

class BaseServer {
public:
    BaseServer(const std::string& serverType);
    int run(int argc, char** argv);
    static void SignalHandlerStatic(int signal);
    SocketServer* getSocketServer() { return server; }

    // TCP session management hooks
    virtual void onClientConnected(intptr_t clientSock);
    virtual void onClientDisconnected(intptr_t clientSock);
    virtual void handlePacket(const PacketHeader& header, const std::vector<uint8_t>& data, intptr_t clientSock) = 0;

    void sendToClient(const void* packet, size_t size, intptr_t clientSock);

    // Heartbeat constants
    static constexpr int HEARTBEAT_INTERVAL_SEC = 5;
    static constexpr int HEARTBEAT_TIMEOUT_SEC = 12;

protected:
    bool loadConfig(int argc, char** argv);
    bool startSocket();
    void connectRedisWithRetry();
    void connectMySQLWithRetry(); // NEW
    void mainLoop();
    void PrintMySQLDiagnostics(); // NEW: MySQL diagnostics

    static std::atomic<bool> running;
    std::string serverType;
    SocketPacketHandler packetHandler;
    Config config;
    std::string configPath;
    SocketServer* server = nullptr;
    RedisClient redis;
    MySQLClient mysql; // NEW: shared MySQL client for all servers
    std::string uniqueId;
    std::string redisKey;
    int num_sessions;
    bool mysqlConnected = false; // NEW: shared connection state
    std::unordered_map<intptr_t, SessionInfo> sessionMap; // Track connected clients and heartbeat

    static std::string GenerateUniqueId();
    static void RegisterServerInRedis(RedisClient& redis, const std::string& key, const std::string& ip, int port, int num_sessions);

    // Heartbeat helpers
    void handleHeartbeatPacket(const std::vector<uint8_t>& data, intptr_t clientSock);
    void checkHeartbeatTimeouts();
};

// Inline static member definition for header-only usage
inline std::atomic<bool> BaseServer::running{true};
