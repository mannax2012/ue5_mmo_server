#include "../../common/include/Config.h"
#include "../../common/include/MySQLClient.h"
#include "../../common/include/RedisClient.h"
#include "../../common/include/Packets.h"
#include "../../common/include/SocketServer.h"
#include "../../common/include/BaseServer.h"
#include "../../common/include/Log.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <csignal>
#include <atomic>
#include <unordered_map>
#include <random>
#include <map>
#include <boost/asio.hpp>

struct SessionInfo {
    int32_t playerId;
    int32_t charId = -1;
    std::string username;
    std::string sessionKey;
    intptr_t clientSock;
};

class AuthServer : public BaseServer {
public:
    AuthServer() : BaseServer("auth") {}

    void handlePacket(const std::vector<uint8_t>& data, intptr_t clientSock) override {
        if (data.size() < sizeof(PacketHeader)) return;
        PacketHeader header;
        std::memcpy(&header, data.data(), sizeof(PacketHeader));
        SessionInfo& session = sessionMap_[clientSock];
        switch (header.packetId) {
            case PACKET_C_LOGIN_REQUEST: {
                C_LoginRequest req;
                if (data.size() < sizeof(C_LoginRequest)) return;
                std::memcpy(&req, data.data(), sizeof(C_LoginRequest));
                std::string username(req.username, req.usernameLength);
                std::string password(req.password, req.passwordLength);
                LOG_DEBUG("Received C_LoginRequest from fd=" + std::to_string(clientSock) + " user='" + username + "' (len=" + std::to_string(req.usernameLength) + ")");
                {
                    std::ostringstream oss;
                    oss << "Username hex: ";
                    for (int i = 0; i < req.usernameLength; ++i) oss << std::hex << (int)(unsigned char)req.username[i] << " ";
                    LOG_DEBUG(oss.str());
                }
                std::string query = "SELECT id, password FROM accounts WHERE username='" + username + "'";
                LOG_DEBUG("Query: " + query);
                std::vector<std::vector<std::string>> result;
                bool queryResult = mysql.query(query, result);
                LOG_DEBUG(std::string("mysql.query returned: ") + (queryResult ? "true" : "false") + ", result.size(): " + std::to_string(result.size()));
                if (!queryResult) {
                    LOG_DEBUG(std::string("MySQL query failed. Error: ") + mysql.getLastError());
                    PrintMySQLDiagnostics();
                }
                if (!queryResult || result.empty()) {
                    LOG_DEBUG("Account not found: " + username);
                    S_LoginResponse resp{};
                    resp.header.packetId = PACKET_S_LOGIN_RESPONSE;
                    resp.resultCode = 1;
                    resp.sessionKeyLength = 0;
                    sendToClient(&resp, sizeof(resp), clientSock);
                    break;
                }
                std::string dbPassword = result[0][1];
                int32_t playerId = std::stoi(result[0][0]);
                if (dbPassword != password) {
                    LOG_DEBUG("Password mismatch for: " + username);
                    S_LoginResponse resp{};
                    resp.header.packetId = PACKET_S_LOGIN_RESPONSE;
                    resp.resultCode = 1;
                    resp.sessionKeyLength = 0;
                    sendToClient(&resp, sizeof(resp), clientSock);
                    break;
                }
                // Success: create session
                std::string sessionKey = GenerateSessionKey();
                std::string redisSessionKey = "session_" + sessionKey;
                std::vector<std::pair<std::string, std::string>> sessionFields = {
                    {"playerId", std::to_string(playerId)},
                    {"username", username},
                    {"charId", std::to_string(session.charId)},
                    {"sessionKey", sessionKey}
                };
                redis.hset(redisSessionKey, sessionFields);
                session.playerId = playerId;
                session.username = username;
                session.sessionKey = sessionKey;
                S_LoginResponse resp{};
                resp.header.packetId = PACKET_S_LOGIN_RESPONSE;
                resp.resultCode = 0;
                resp.sessionKeyLength = (int8_t)sessionKey.size();
                std::memset(resp.sessionKey, 0, sizeof(resp.sessionKey));
                std::memcpy(resp.sessionKey, sessionKey.data(), sessionKey.size());
                sendToClient(&resp, sizeof(resp), clientSock);
            }
            break;
            case PACKET_C_CHAR_CREATE: {
                C_CharCreate req;
                if (DeserializePacketRaw(data, &req, sizeof(C_CharCreate))) {
                    LOG_DEBUG("Received C_CharCreate from fd=" + std::to_string(clientSock));
                    std::string charName(req.name, req.nameLength);
                    int16_t classId = req.classId;
                    LOG_DEBUG("CharCreate request: name='" + charName + "', classId=" + std::to_string(classId) + ", playerId=" + std::to_string(session.playerId));

                    // 1. Validate name (length, allowed chars)
                    if (charName.empty() || charName.length() > 31) {
                        LOG_DEBUG("CharCreate failed: invalid name length");
                        S_CharCreateResult resp{};
                        resp.header.packetId = PACKET_S_CHAR_CREATE_RESULT;
                        resp.resultCode = 1; // fail
                        resp.charId = 0;
                        sendToClient(&resp, sizeof(resp), clientSock);
                        break;
                    }
                    // 2. Check for duplicate name for this account
                    std::string checkQuery = "SELECT id FROM characters WHERE name='" + charName + "'";
                    std::vector<std::vector<std::string>> checkResult;
                    bool checkOk = mysql.query(checkQuery, checkResult);
                    if (!checkOk) {
                        LOG_DEBUG("[MYSQL] error during char name check: " + mysql.getLastError());
                        S_CharCreateResult resp{};
                        resp.header.packetId = PACKET_S_CHAR_CREATE_RESULT;
                        resp.resultCode = 1; // fail
                        resp.charId = 0;
                        sendToClient(&resp, sizeof(resp), clientSock);
                        break;
                    }
                    if (!checkResult.empty()) {
                        LOG_DEBUG("[MYSQL] CharCreate failed: duplicate name");
                        S_CharCreateResult resp{};
                        resp.header.packetId = PACKET_S_CHAR_CREATE_RESULT;
                        resp.resultCode = 1; // fail
                        resp.charId = 0;
                        sendToClient(&resp, sizeof(resp), clientSock);
                        break;
                    }
                    // 3. Insert new character
                    std::vector<std::string> columns = {"account_id", "name", "class_id"};
                    std::vector<std::string> values = {std::to_string(session.playerId), charName, std::to_string(classId)};
                    bool insertOk = mysql.insert("characters", columns, values);
                    if (!insertOk) {
                        LOG_DEBUG("[MYSQL] error during char insert: " + mysql.getLastError());
                        S_CharCreateResult resp{};
                        resp.header.packetId = PACKET_S_CHAR_CREATE_RESULT;
                        resp.resultCode = 1; // fail
                        resp.charId = 0;
                        sendToClient(&resp, sizeof(resp), clientSock);
                        break;
                    }
                    // 4. Get new character ID
                    std::string idQuery = "SELECT id FROM characters WHERE name='" + charName + "' AND account_id=" + std::to_string(session.playerId);
                    std::vector<std::vector<std::string>> idResult;
                    int32_t charId = 0;
                    if (mysql.query(idQuery, idResult) && !idResult.empty()) {
                        charId = std::stoi(idResult[0][0]);
                    } else {
                        LOG_DEBUG("[MYSQL] error: could not retrieve new charId after insert");
                        S_CharCreateResult resp{};
                        resp.header.packetId = PACKET_S_CHAR_CREATE_RESULT;
                        resp.resultCode = 1; // fail
                        resp.charId = 0;
                        sendToClient(&resp, sizeof(resp), clientSock);
                        break;
                    }
                    // 5. Success response
                    LOG_DEBUG("CharCreate success: name='" + charName + "', charId=" + std::to_string(charId));
                    S_CharCreateResult resp{};
                    resp.header.packetId = PACKET_S_CHAR_CREATE_RESULT;
                    resp.resultCode = 0; // success
                    resp.charId = charId;
                    sendToClient(&resp, sizeof(resp), clientSock);
                }
                break;
            }
            case PACKET_C_CHAR_SELECT: {
                C_CharSelect req;
                if (DeserializePacketRaw(data, &req, sizeof(C_CharSelect))) {
                    LOG_DEBUG("Received C_CharSelect from " + std::to_string(clientSock));
                    int32_t charId = req.charId;
                    // Check if character belongs to this account
                    std::string query = "SELECT id FROM characters WHERE id=" + std::to_string(charId) + " AND account_id=" + std::to_string(session.playerId);
                    std::vector<std::vector<std::string>> result;
                    bool ok = mysql.query(query, result);
                    S_CharSelectResult resp{};
                    resp.header.packetId = PACKET_S_CHAR_SELECT_RESULT;
                    if (!ok) {
                        LOG_DEBUG("[MYSQL] error during char select: " + mysql.getLastError());
                        resp.resultCode = 1; // fail
                        std::memset(resp.gameServerAddress, 0, sizeof(resp.gameServerAddress));
                        resp.gameServerPort = 0;
                        sendToClient(&resp, sizeof(resp), clientSock);
                        break;
                    }
                    if (result.empty()) {
                        LOG_DEBUG("CharSelect failed: character does not belong to account or does not exist");
                        resp.resultCode = 1; // fail
                        std::memset(resp.gameServerAddress, 0, sizeof(resp.gameServerAddress));
                        resp.gameServerPort = 0;
                        sendToClient(&resp, sizeof(resp), clientSock);
                        break;
                    }
                    // Success: update the session and assign a game server (for now, use config values)
                    std::string gameIp = config.get("game_server_ip", "127.0.0.1");
                    int gamePort = config.getInt("game_server_port", 4000);

                    session.charId = charId;

                    std::string redisSessionKey = "session_" + session.sessionKey;
                    std::vector<std::pair<std::string, std::string>> sessionFields = {
                        {"playerId", std::to_string(session.playerId)},
                        {"username", session.username},
                        {"charId", std::to_string(session.charId)},
                        {"sessionKey", session.sessionKey},
                        {"fd", std::to_string(clientSock)}
                    };
                    redis.hset(redisSessionKey, sessionFields);
                    resp.resultCode = 0;
                    std::memset(resp.gameServerAddress, 0, sizeof(resp.gameServerAddress));
                    std::strncpy(resp.gameServerAddress, gameIp.c_str(), sizeof(resp.gameServerAddress) - 1);
                    resp.gameServerPort = gamePort;
                    LOG_DEBUG("CharSelect success: charId=" + std::to_string(charId) + ", gameServer=" + gameIp + ":" + std::to_string(gamePort));
                    sendToClient(&resp, sizeof(resp), clientSock);
                }
                break;
            }
            case PACKET_C_CHAR_DELETE: {
                C_CharDelete req;
                if (DeserializePacketRaw(data, &req, sizeof(C_CharDelete))) {
                    LOG_DEBUG("Received C_CharDelete from " + std::to_string(clientSock));
                    int32_t charId = req.charId;
                    // Check if character belongs to this account
                    std::string query = "SELECT id FROM characters WHERE id=" + std::to_string(charId) + " AND account_id=" + std::to_string(session.playerId);
                    std::vector<std::vector<std::string>> result;
                    bool ok = mysql.query(query, result);
                    S_CharDeleteResult resp{};
                    resp.header.packetId = PACKET_S_CHAR_DELETE_RESULT;
                    resp.charId = charId;
                    if (!ok) {
                        LOG_DEBUG("[MYSQL] error during char delete check: " + mysql.getLastError());
                        resp.resultCode = 1; // fail
                        sendToClient(&resp, sizeof(resp), clientSock);
                        break;
                    }
                    if (result.empty()) {
                        LOG_DEBUG("CharDelete failed: character does not belong to account or does not exist");
                        resp.resultCode = 1; // fail
                        sendToClient(&resp, sizeof(resp), clientSock);
                        break;
                    }
                    // Delete character
                    std::string delQuery = "DELETE FROM characters WHERE id=" + std::to_string(charId) + " AND account_id=" + std::to_string(session.playerId);
                    bool delOk = mysql.exec(delQuery);
                    if (!delOk) {
                        LOG_DEBUG("[MYSQL] error during char delete: " + mysql.getLastError());
                        resp.resultCode = 1; // fail
                        sendToClient(&resp, sizeof(resp), clientSock);
                        break;
                    }
                    LOG_DEBUG("CharDelete success: charId=" + std::to_string(charId));
                    resp.resultCode = 0; // success
                    sendToClient(&resp, sizeof(resp), clientSock);
                }
                break;
            }
            case PACKET_C_CHAR_LIST_REQUEST: {
                C_CharListRequest req;
                if (DeserializePacketRaw(data, &req, sizeof(C_CharListRequest))) {
                    LOG_DEBUG("Received C_CharListRequest from fd=" + std::to_string(clientSock));
                    // Query all characters for this account
                    std::string query = "SELECT id, name, class_id FROM characters WHERE account_id=" + std::to_string(session.playerId);
                    std::vector<std::vector<std::string>> result;
                    bool ok = mysql.query(query, result);
                    S_CharListResult resp{};
                    resp.header.packetId = PACKET_S_CHAR_LIST_RESULT;
                    resp.charCount = 0;
                    if (!ok) {
                        LOG_DEBUG("[MYSQL] error during char list: " + mysql.getLastError());
                        sendToClient(&resp, sizeof(resp), clientSock);
                        break;
                    }
                    int count = std::min((int)result.size(), 8); // max 8 chars
                    resp.charCount = static_cast<int8_t>(count);
                    for (int i = 0; i < count; ++i) {
                        resp.entries[i].charId = std::stoi(result[i][0]);
                        std::memset(resp.entries[i].name, 0, sizeof(resp.entries[i].name));
                        std::strncpy(resp.entries[i].name, result[i][1].c_str(), sizeof(resp.entries[i].name) - 1);
                        resp.entries[i].classId = static_cast<int16_t>(std::stoi(result[i][2]));
                    }
                    LOG_DEBUG("CharListResult: " + std::to_string(count) + " chars for playerId=" + std::to_string(session.playerId));
                    sendToClient(&resp, sizeof(resp), clientSock);
                }
                break;
            }
            default:
                LOG_DEBUG("Unknown or unhandled packet: " + std::to_string(header.packetId) + " from fd=" + std::to_string(clientSock));
                break;
        }
    }

    std::string GenerateSessionKey() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<uint64_t> dis;
        uint64_t id = dis(gen);
        char buf[32];
        snprintf(buf, sizeof(buf), "%016llx", (unsigned long long)id);
        return std::string(buf);
    }

private:
    std::map<intptr_t, SessionInfo> sessionMap_;
    RedisClient redis;
};

int main(int argc, char** argv) {
    boost::asio::io_context io_context;
    boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
    signals.async_wait([&](const boost::system::error_code& ec, int signal_num){
        LOG_INFO("Signal received, shutting down... (signal=" + std::to_string(signal_num) + ", ec=" + ec.message() + ")");
        BaseServer::SignalHandlerStatic(signal_num);
        io_context.stop();
    });
    AuthServer server;
    // Start server thread
    std::thread server_thread([&](){
        LOG_DEBUG("server_thread: starting server.run()");
        server.run(argc, argv);
        LOG_DEBUG("server_thread: server.run() returned, thread exiting.");
    });
    // Run io_context in main thread
    io_context.run();
    LOG_DEBUG("io_context ended run.");
    // After signal, wait for server to finish
    server_thread.join();
    LOG_DEBUG("Server Thread joined.");
    // Now stop io_context to ensure all handlers are done
    io_context.stop();
    LOG_DEBUG("main() is returning, process should exit now.");
    std::_Exit(0);
    return 0;
}
