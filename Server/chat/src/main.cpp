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

std::atomic<bool> running{true};

void SignalHandler(int signal) {
    running = false;
}

class ChatServer : public BaseServer {
public:
    ChatServer() : BaseServer("chat") {}

    void handlePacket(const PacketHeader& header, const std::vector<uint8_t>& data, intptr_t clientSock) override {
        switch (header.packetId) {
            case PACKET_C_CHAT_MESSAGE: {
                C_ChatMessage req;
                if (data.size() < sizeof(C_ChatMessage)) return;
                std::memcpy(&req, data.data(), sizeof(C_ChatMessage));
                LOG_DEBUG(std::string("Received C_ChatMessage from socket ") + std::to_string(clientSock));
                // TODO: Handle chat message logic
                break;
            }
            default:
                LOG_DEBUG(std::string("Unknown or unhandled packet: ") + std::to_string(header.packetId) + " from socket " + std::to_string(clientSock));
                break;
        }
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
    ChatServer server;
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
