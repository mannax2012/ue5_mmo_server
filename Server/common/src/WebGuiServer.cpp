#include "../include/WebGuiServer.h"
#include "../include/BaseServer.h"
#include "../include/Log.h"
#include "../../game/include/Player.h"
#include "../../game/include/GameServer.h"
#include <chrono>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/signal_set.hpp>
#include "../include/Log.h"
#include <iostream>
#include <nlohmann/json.hpp>
#include <fstream>
#include <streambuf>
#include "../webgui/index_html.h"

#include <sys/utsname.h>
#include <boost/algorithm/string.hpp>

using tcp = boost::asio::ip::tcp;
namespace http = boost::beast::http;

WebGuiServer::WebGuiServer(BaseServer* server, int port)
    : port(port), server(server), running(false) {}

WebGuiServer::~WebGuiServer() { Stop(); }

void WebGuiServer::Start() {
    if (running) return;
    running = true;
    serverThread = std::thread([this]() { Run(); });
}

void WebGuiServer::Stop() {
    running = false;
    if (serverThread.joinable()) serverThread.join();
}

void WebGuiServer::Run() {
    boost::asio::io_context ioc;
    tcp::acceptor acceptor(ioc, {tcp::v4(), (unsigned short)port});
    while (running) {
        try {
            tcp::socket socket(ioc);
            acceptor.accept(socket);
            boost::beast::tcp_stream stream(std::move(socket));
            boost::beast::flat_buffer buffer;
            http::request<http::string_body> req;
            http::read(stream, buffer, req);
            HandleRequest(std::move(req), std::move(stream));
        } catch (const boost::beast::system_error& se) {
            if (se.code() == boost::beast::http::error::end_of_stream || se.code() == boost::asio::error::eof) {
                // Client closed connection, just continue
                continue;
            } else {
                LOG_WARNING(std::string("WebGuiServer Beast system_error: ") + se.code().message());
            }
        } catch (const std::exception& ex) {
            LOG_ERROR(std::string("WebGuiServer exception: ") + ex.what());
        }
    }
}

void WebGuiServer::HandleRequest(http::request<http::string_body>&& req, boost::beast::tcp_stream&& stream) {
    std::string target = std::string(req.target());
    RouteRequest(target, std::move(req), std::move(stream));
}

bool WebGuiServer::isIpAllowed(const std::string& remoteIp) {
    // Get allowed IPs from config (comma-separated, e.g. "127.0.0.1,192.168.1.1,*")
    std::string allowed = server ? server->GetConfig().get("webgui_allowed_ips", "127.0.0.1") : "127.0.0.1";
    std::vector<std::string> allowedList;
    boost::split(allowedList, allowed, boost::is_any_of(","));
    for (auto& ip : allowedList) boost::trim(ip);
    for (const auto& ip : allowedList) {
        if (ip == "*" || ip == remoteIp) return true;
    }
    return false;
}
// Minimal portable base64 decoder for HTTP Basic Auth
typedef unsigned char uchar;
static std::string base64_decode(const std::string& in) {
    std::string out;
    std::vector<int> T(256, -1);
    for (int i = 0; i < 64; i++) T["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[i]] = i;
    int val = 0, valb = -8;
    for (uchar c : in) {
        if (T[c] == -1) break;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            out.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return out;
}

bool WebGuiServer::isAuthOk(const http::request<http::string_body>& req) {
    std::string expectedUser = server ? server->GetConfig().get("webgui_user", "admin") : "admin";
    std::string expectedPass = server ? server->GetConfig().get("webgui_password", "admin") : "admin";
    auto auth = req[http::field::authorization];
    if (auth.substr(0,6) == "Basic ") {
        std::string encoded = auth.substr(6);
        std::string decoded;
        try {
            decoded = base64_decode(encoded);
        } catch (...) { return false; }
        auto sep = decoded.find(":");
        if (sep != std::string::npos) {
            std::string user = decoded.substr(0, sep);
            std::string pass = decoded.substr(sep+1);
            return (user == expectedUser && pass == expectedPass);
        }
    }
    return false;
}

void WebGuiServer::RouteRequest(const std::string& target, http::request<http::string_body>&& req, boost::beast::tcp_stream&& stream) {
    // Get remote IP
    std::string remoteIp = "unknown";
    try {
        auto ep = stream.socket().remote_endpoint();
        remoteIp = ep.address().to_string();
    } catch (...) {}
    if (!isIpAllowed(remoteIp)) {
        http::response<http::string_body> res{http::status::forbidden, req.version()};
        res.set(http::field::server, "MMO-Server-WebGUI");
        res.set(http::field::content_type, "text/plain");
        res.body() = "403 Forbidden: IP not allowed";
        res.prepare_payload();
        http::write(stream, res);
        return;
    }
    if (!isAuthOk(req)) {
        http::response<http::string_body> res{http::status::unauthorized, req.version()};
        res.set(http::field::server, "MMO-Server-WebGUI");
        res.set(http::field::content_type, "text/plain");
        res.set(http::field::www_authenticate, "Basic realm=\"MMO WebGUI\"");
        res.body() = "401 Unauthorized: Provide valid credentials";
        res.prepare_payload();
        http::write(stream, res);
        return;
    }
    if (target == "/sessions") {
        HandleSessions(std::move(req), std::move(stream));
    } else if (target == "/shards") {
        HandleShards(std::move(req), std::move(stream));
    } else if (target == "/entities") {
        HandleEntities(std::move(req), std::move(stream));
    } else if (target.rfind("/logs", 0) == 0) { // allow /logs?filter=...
        HandleLogs(std::move(req), std::move(stream));
    } else if (target == "/dashboard") {
        HandleDashboard(std::move(req), std::move(stream));
    } else if (target == "/" || target == "/index.html") {
        HandleFrontend(std::move(req), std::move(stream));
    } else {
        http::response<http::string_body> res{http::status::not_found, req.version()};
        res.set(http::field::server, "MMO-Server-WebGUI");
        res.set(http::field::content_type, "text/plain");
        res.body() = "404 Not Found";
        res.prepare_payload();
        http::write(stream, res);
    }
}
// --- Dashboard handler ---
void WebGuiServer::HandleDashboard(http::request<http::string_body>&& req, boost::beast::tcp_stream&& stream) {
    nlohmann::json j;
    // Server type
    j["serverType"] = server ? server->serverType : "unknown";
    // Server port
    j["port"] = port;
    // Server IP (try to get from config or socket)
    std::string ip = "unknown";
    if (server) {
        ip = server->GetConfig().get("bind_ip", "0.0.0.0");
    }
    j["ip"] = ip;
    // Uptime
    static auto start_time = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    auto uptime_sec = std::chrono::duration_cast<std::chrono::seconds>(now - start_time).count();
    j["uptime_sec"] = uptime_sec;
    // OS info
#ifdef _WIN32
    j["os"] = "Windows";
#else
    struct utsname uts;
    if (uname(&uts) == 0) {
        j["os"] = std::string(uts.sysname) + " " + uts.release;
    } else {
        j["os"] = "Unknown";
    }
#endif
    // CPU and RAM info (Linux only)
#ifdef __linux__
    // CPU info
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line, model;
    int cores = 0;
    while (std::getline(cpuinfo, line)) {
        if (line.find("model name") != std::string::npos && model.empty()) {
            auto pos = line.find(":");
            if (pos != std::string::npos) model = line.substr(pos + 2);
        }
        if (line.find("processor") != std::string::npos) ++cores;
    }
    j["cpu_model"] = model;
    j["cpu_cores"] = cores;
    // RAM info
    std::ifstream meminfo("/proc/meminfo");
    long mem_total = 0, mem_free = 0;
    while (std::getline(meminfo, line)) {
        if (line.find("MemTotal") != std::string::npos) {
            sscanf(line.c_str(), "MemTotal: %ld kB", &mem_total);
        }
        if (line.find("MemAvailable") != std::string::npos) {
            sscanf(line.c_str(), "MemAvailable: %ld kB", &mem_free);
        }
    }
    j["ram_total_kb"] = mem_total;
    j["ram_available_kb"] = mem_free;
#endif
    http::response<http::string_body> res{http::status::ok, req.version()};
    res.set(http::field::server, "MMO-Server-WebGUI");
    res.set(http::field::content_type, "application/json");
    res.body() = j.dump();
    res.prepare_payload();
    http::write(stream, res);
}


void WebGuiServer::HandleSessions(http::request<http::string_body>&& req, boost::beast::tcp_stream&& stream) {
    nlohmann::json j;
    if (server && server->IsWebGuiEnabled()) {
        auto& sessions = server->GetSessionMap();
        for (const auto& kv : sessions) {
            const auto& key = kv.first;
            const auto& info = kv.second;
            nlohmann::json entry = {
                {"endpoint", key},
                {"connected", info.connected},
                {"lastHeartbeat", std::chrono::duration_cast<std::chrono::seconds>(info.lastHeartbeat.time_since_epoch()).count()},
                {"playerId", info.playerId},
                {"charId", info.charId},
                {"username", info.username},
                {"sessionKey", info.sessionKey}
            };
            if (info.playerEntity) {
                entry["playerName"] = info.playerEntity->name;
            }
            j["sessions"].push_back(entry);
        }
    } else {
        j["sessions"] = nlohmann::json::array();
    }
    http::response<http::string_body> res{http::status::ok, req.version()};
    res.set(http::field::server, "MMO-Server-WebGUI");
    res.set(http::field::content_type, "application/json");
    res.body() = j.dump();
    res.prepare_payload();
    http::write(stream, res);
}

void WebGuiServer::HandleShards(http::request<http::string_body>&& req, boost::beast::tcp_stream&& stream) {
    nlohmann::json j;
#if defined(GAME_SERVER) // Only include this logic for the game server build
    if (server && server->IsWebGuiEnabled()) {
        auto* gameServer = dynamic_cast<GameServer*>(server);
        if (gameServer) {
            auto& zoneManager = gameServer->zoneManager;
            nlohmann::json shards = nlohmann::json::array();
            for (const auto& tup : zoneManager.GetZoneSummary()) {
                int32_t zoneId;
                int entityCount, playerCount;
                std::tie(zoneId, entityCount, playerCount) = tup;
                shards.push_back({
                    {"zoneId", zoneId},
                    {"entityCount", entityCount},
                    {"playerCount", playerCount}
                });
            }
            j["shards"] = shards;
        } else {
            j["shards"] = nlohmann::json::array();
        }
    } else {
        j["shards"] = nlohmann::json::array();
    }
#else
    j["shards"] = nlohmann::json::array();
#endif
    http::response<http::string_body> res{http::status::ok, req.version()};
    res.set(http::field::server, "MMO-Server-WebGUI");
    res.set(http::field::content_type, "application/json");
    res.body() = j.dump();
    res.prepare_payload();
    http::write(stream, res);
}

void WebGuiServer::HandleEntities(http::request<http::string_body>&& req, boost::beast::tcp_stream&& stream) {
    nlohmann::json j;
    if (server && server->IsWebGuiEnabled()) {
        // Try to get MobManager from server (dynamic_cast for GameServer)
        // This is a stub: you may want to expose a virtual GetMobManager() in BaseServer for real use
        // For now, just return empty
        j["entities"] = nlohmann::json::array();
    } else {
        j["entities"] = nlohmann::json::array();
    }
    http::response<http::string_body> res{http::status::ok, req.version()};
    res.set(http::field::server, "MMO-Server-WebGUI");
    res.set(http::field::content_type, "application/json");
    res.body() = j.dump();
    res.prepare_payload();
    http::write(stream, res);
}

void WebGuiServer::HandleLogs(http::request<http::string_body>&& req, boost::beast::tcp_stream&& stream) {
    nlohmann::json j;
    if (server && server->IsWebGuiEnabled()) {
        // Parse query parameters for filtering
        std::string target = std::string(req.target());
        size_t qpos = target.find('?');
        std::string text_filter;
        LogLevel min_level = LogLevel::DEBUG;
        size_t max_count = 100;
        if (qpos != std::string::npos) {
            std::string query = target.substr(qpos + 1);
            std::istringstream iss(query);
            std::string kv;
            while (std::getline(iss, kv, '&')) {
                size_t eq = kv.find('=');
                if (eq != std::string::npos) {
                    std::string key = kv.substr(0, eq);
                    std::string val = kv.substr(eq + 1);
                    if (key == "filter") text_filter = val;
                    else if (key == "level") min_level = LogLevelFromString(val);
                    else if (key == "count") max_count = std::stoul(val);
                }
            }
        }
        auto logs = GetRecentLogs(max_count, min_level, text_filter);
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& entry : logs) {
            arr.push_back({
                {"timestamp", entry.timestamp},
                {"time", FormatTimestamp(entry.timestamp)},
                {"level", LogLevelToString(entry.level)},
                {"message", entry.message}
            });
        }
        j["logs"] = arr;
    } else {
        j["logs"] = nlohmann::json::array();
    }
    http::response<http::string_body> res{http::status::ok, req.version()};
    res.set(http::field::server, "MMO-Server-WebGUI");
    res.set(http::field::content_type, "application/json");
    res.body() = j.dump();
    res.prepare_payload();
    http::write(stream, res);
}

void WebGuiServer::HandleFrontend(http::request<http::string_body>&& req, boost::beast::tcp_stream&& stream) {
    http::response<http::string_body> res{http::status::ok, req.version()};
    res.set(http::field::server, "MMO-Server-WebGUI");
    res.set(http::field::content_type, "text/html");
    res.body() = EMBEDDED_INDEX_HTML;
    res.prepare_payload();
    http::write(stream, res);
}
