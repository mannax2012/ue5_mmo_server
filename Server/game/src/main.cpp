#include "../../common/include/Config.h"
#include "../../common/include/MySQLClient.h"
#include "../../common/include/RedisClient.h"
#include "../../common/include/Packets.h"
#include "../../common/include/SocketServer.h"
#include "../../common/include/BaseServer.h"
#include "../../common/include/Log.h"
#include "MobManager.h"
#include "ZoneManager.h"
#include "Player.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <csignal>
#include <atomic>
#include <unordered_map>
#include <random>
#include <map>
#include <boost/asio.hpp>

class GameServer : public BaseServer {
  public:
    GameServer() : BaseServer("game") {}    
    MobManager mobManager;
    ZoneManager zoneManager;
 

    void handlePacket(const PacketHeader& header, const std::vector<uint8_t>& data, intptr_t clientSock) override {
        // Use BaseServer's sessionMap for session tracking
        auto& session = sessionMap[clientSock];

        //CHECK FOR LOGIN
        if(session.playerId == -1 && header.packetId != PACKET_C_CONNECT_REQUEST) {
            LOG_DEBUG("Session not logged in, dropping packet.");
            return;
        }

        switch (header.packetId) {
            case PACKET_C_CONNECT_REQUEST: {
                C_ConnectRequest req;
                if (data.size() < sizeof(C_ConnectRequest)) return;
                std::memcpy(&req, data.data(), sizeof(C_ConnectRequest));

                LOG_DEBUG(std::string("Received C_ConnectRequest from socket ") + std::to_string(clientSock));

                // Extract sessionKey from packet (ensure null-termination)
                std::string sessionKey(req.sessionKey, strnlen(req.sessionKey, sizeof(req.sessionKey)));
                std::string redisSessionKey = "session_" + sessionKey;
                std::map<std::string, std::string> sessionData;
                bool redisOk = redis.hgetall(redisSessionKey, sessionData);
                if (!redisOk || sessionData.empty()) {
                    LOG_ERROR("Session not found in Redis for key: " + redisSessionKey);
                    // Optionally disconnect or send error to client
                    S_ConnectResult resp{};
                    resp.header.packetId = PACKET_S_CONNECT_RESULT;
                    resp.resultCode = 1; // fail
                    sendToClient(&resp, sizeof(resp), clientSock);
                    server->disconnect(clientSock);
                    return;
                }
                // Update session in sessionMap (SessionInfo struct)
                if (sessionData.count("playerId")) session.playerId = std::stoi(sessionData["playerId"]);
                if (sessionData.count("charId")) session.charId = std::stoi(sessionData["charId"]);
                if (sessionData.count("username")) session.username = sessionData["username"];
                if (sessionData.count("sessionKey")) session.sessionKey = sessionData["sessionKey"];
                else session.sessionKey = sessionKey;

                // Update Redis session with current fd
                std::vector<std::pair<std::string, std::string>> updateFields = {
                    {"fd", std::to_string(clientSock)}
                };
                redis.hset(redisSessionKey, updateFields);

                session.connected = true;
                session.lastHeartbeat = std::chrono::steady_clock::now();
                LOG_DEBUG("Session updated in sessionMap for socket " + std::to_string(clientSock) + ", sessionKey: " + session.sessionKey);
                // Send connect result (success)
                S_ConnectResult resp{};
                resp.header.packetId = PACKET_S_CONNECT_RESULT;
                resp.resultCode = 0; // success
                sendToClient(&resp, sizeof(resp), clientSock);

                // After successful connect, create Player entity and add to ZoneManager
                if (resp.resultCode == 0) {
                    auto player = std::make_shared<Player>();
                    player->InitFromDB(mysql, session.charId, session.username, session.playerId);
                    zoneManager.AddEntityToZone(player->zoneId, player);
                    session.playerEntity = player;
                }
                break;
            }
            case PACKET_C_MOVE: {
                C_Move req;
                if (data.size() < sizeof(C_Move)) return;
                std::memcpy(&req, data.data(), sizeof(C_Move));
                LOG_DEBUG(std::string("Received C_Move from socket ") + std::to_string(clientSock));
                auto player = session.playerEntity;
                if (player) {
                    player->MoveTo(req.x, req.y, req.z, &zoneManager);
                }
                break;
            }
            case PACKET_C_COMBAT_ACTION: {
                C_CombatAction req;
                if (data.size() < sizeof(C_CombatAction)) return;
                std::memcpy(&req, data.data(), sizeof(C_CombatAction));
                LOG_DEBUG(std::string("Received C_CombatAction from socket ") + std::to_string(clientSock));
                // TODO: Handle combat logic
                break;
            }
            case PACKET_C_SHOP_BUY: {
                C_ShopBuy req;
                if (data.size() < sizeof(C_ShopBuy)) return;
                std::memcpy(&req, data.data(), sizeof(C_ShopBuy));
                LOG_DEBUG(std::string("Received C_ShopBuy from socket ") + std::to_string(clientSock));
                // TODO: Handle shop buy logic
                break;
            }
            default:
                LOG_DEBUG(std::string("Unknown or unhandled packet: ") + std::to_string(header.packetId) + " from socket " + std::to_string(clientSock));
                break;
        }
    }

    int run(int argc, char** argv) {
        int result = BaseServer::run(argc, argv);
        //Initialize Zone Size  
        zoneManager.SetZoneSize(config.getInt("zone_size", 100));
        return result;
    }

};

int main(int argc, char** argv) {
        boost::asio::io_context io_context;
    boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
    signals.async_wait([&](const boost::system::error_code& ec, int signal_num){
        LOG_INFO("Signal received, shutting down... (signal=" + std::to_string(signal_num) + ", ec=" + ec.message() + ")");
        BaseServer::SignalHandlerStatic(signal_num);
        io_context.stop();
    });
    GameServer server;
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
