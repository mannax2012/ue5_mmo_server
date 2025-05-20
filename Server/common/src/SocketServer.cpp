#include "SocketServer.h"
#include <thread>
#include <atomic>
#include <vector>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <cstring>
#include <boost/asio.hpp>
#include <iostream>
#include <map>
#include <unordered_map>
#include "../include/Log.h"

// High-performance TCP server (cross-platform, non-blocking, multithreaded, failsafe, using Boost.Asio)
class SocketServerImpl : public SocketServer {
public:
    SocketServerImpl() : ioContext(), acceptor(ioContext), running(false) {}
    ~SocketServerImpl() { stop(); }
    bool start(const char* ip, int port) override {
        try {
            boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(ip), port);
            acceptor.open(endpoint.protocol());
            acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
            acceptor.bind(endpoint);
            acceptor.listen();
            running = true;
            doAccept();
            threads.emplace_back([this]() { ioContext.run(); });
            return true;
        } catch (const std::exception& e) {
            LOG_ERROR("SocketServer start error: " + std::string(e.what()));
            return false;
        }
    }
    void stop() override {
        running = false;
        ioContext.stop();
        LOG_DEBUG("Stopping SocketServerImpl..., joining threads...");
        for (auto& t : threads) if (t.joinable()) t.join();
        LOG_DEBUG(" ALL SocketServerImpl threads joined.");
        threads.clear();
    }
    void send(const std::vector<uint8_t>& data, intptr_t clientSock) override {
        auto it = clients.find(clientSock);
        if (it != clients.end()) {
            // Prepend 4-byte length prefix (network byte order)
            uint32_t len = htonl(static_cast<uint32_t>(data.size()));
            std::vector<uint8_t> out;
            out.resize(4 + data.size());
            std::memcpy(out.data(), &len, 4);
            std::memcpy(out.data() + 4, data.data(), data.size());
            // Log the actual outgoing packet (including TCP length prefix)
            {
                std::ostringstream oss;
                oss << "[ACTUAL OUT] fd=" << clientSock << ", size=" << out.size() << ": ";
                for (size_t i = 0; i < out.size(); ++i) oss << std::hex << (int)out[i] << " ";
                LOG_DEBUG_EXT(oss.str());
            }
            boost::asio::async_write(*it->second, boost::asio::buffer(out),
                [](const boost::system::error_code&, std::size_t){});
        }
    }
    void setPacketHandler(SocketPacketHandler handler) override {
        this->handler = handler;
    }
    void setConnectionHandler(SocketConnectionHandler onConnect, SocketConnectionHandler onDisconnect) override {
        this->onConnect = onConnect;
        this->onDisconnect = onDisconnect;
    }
    void disconnect(intptr_t clientSock) override {
        auto it = clients.find(clientSock);
        if (it != clients.end()) {
            boost::system::error_code ec;
            it->second->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
            it->second->close(ec);
            clients.erase(it);
            if (onDisconnect) onDisconnect(clientSock);
            LOG_DEBUG("SocketServerImpl: Disconnected client fd=" + std::to_string(clientSock));
        }
    }
private:
    void doAccept() {
        auto socket = std::make_shared<boost::asio::ip::tcp::socket>(ioContext);
        acceptor.async_accept(*socket, [this, socket](const boost::system::error_code& ec) {
            if (!ec && running) {
                intptr_t sockId = reinterpret_cast<intptr_t>(socket.get());
                clients[sockId] = socket;
                if (onConnect) onConnect(sockId);
                doRead(socket, sockId);
            }
            if (running) doAccept();
        });
    }
    void doRead(std::shared_ptr<boost::asio::ip::tcp::socket> socket, intptr_t sockId) {
        auto buf = std::make_shared<std::vector<uint8_t>>(2048);
        socket->async_read_some(boost::asio::buffer(*buf),
            [this, socket, buf, sockId](const boost::system::error_code& ec, std::size_t n) {
                if (!ec && n > 0) {
                    // Accumulate data in per-client buffer
                    auto& buffer = recvBuffers[sockId];
                    buffer.insert(buffer.end(), buf->begin(), buf->begin() + n);
                    // Process all complete packets in the buffer
                    size_t offset = 0;
                    while (buffer.size() - offset >= 4) {
                        uint32_t packetLen = 0;
                        std::memcpy(&packetLen, buffer.data() + offset, 4);
                        packetLen = ntohl(packetLen);
                        if (buffer.size() - offset < 4 + packetLen)
                            break; // Not enough data for a full packet
                        // Extract only the payload (without length prefix)
                        std::vector<uint8_t> payload(buffer.begin() + offset + 4, buffer.begin() + offset + 4 + packetLen);
                        if (handler) handler(payload, sockId);
                        offset += 4 + packetLen;
                    }
                    // Remove processed bytes
                    if (offset > 0) buffer.erase(buffer.begin(), buffer.begin() + offset);
                    doRead(socket, sockId);
                } else {
                    clients.erase(sockId);
                    recvBuffers.erase(sockId);
                    if (onDisconnect) onDisconnect(sockId);
                }
            });
    }
    boost::asio::io_context ioContext;
    boost::asio::ip::tcp::acceptor acceptor;
    std::atomic<bool> running;
    std::vector<std::thread> threads;
    std::map<intptr_t, std::shared_ptr<boost::asio::ip::tcp::socket>> clients;
    std::unordered_map<intptr_t, std::vector<uint8_t>> recvBuffers; // Per-client receive buffer
    SocketPacketHandler handler;
    SocketConnectionHandler onConnect;
    SocketConnectionHandler onDisconnect;
};

SocketServer* CreateSocketServer() { return new SocketServerImpl(); }
