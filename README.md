# MMO Server + Unreal Client Plugin

Distributed MMO backend in C++17 with separate Auth, Game, and Chat services, plus an Unreal Engine runtime client plugin.

## Project status

> ⚠️ **Work in progress:** This project is not finished and is under heavy active development.
> Expect breaking changes, incomplete systems, and missing hardening for production use.

## What this project includes

- **Auth server**: login, character create/select/delete/list, session key handling
- **Game server**: world entities, zone management, movement/combat packets, persistence hooks
- **Chat server**: base packet handling for chat channel flow
- **Shared common layer**: sockets, config parsing, packet contracts, logging, Redis/MySQL integration, security/compression helpers
- **Web GUI monitoring**: lightweight HTTP monitoring endpoints and embedded dashboard page
- **Unreal plugin**: `MMOClient` runtime networking module for game-side integration

## Currently working features

- Multi-service backend binaries build and run (`auth_server`, `game_server`, `chat_server`)
- Config-driven startup using `--config` and key-value config files
- Shared packet layer with compression/encryption path in the common networking stack
- Redis + MySQL connectivity in the shared server layer
- Session lifecycle and heartbeat timeout handling in `BaseServer`
- Auth flow: login, character create/select/delete/list
- Game connect handoff via session key validation from Redis
- Basic player movement handling after game connect
- Entity/zone management foundation with player load/save on connect/disconnect
- Web GUI server can be enabled from config for monitoring endpoints
- Chat server receives chat packets, but full chat channel/message routing logic is still TODO

## Not working yet / in progress

- Chat message handling logic is not implemented yet (`Server/chat/src/main.cpp` TODO)
- Player-specific runtime update logic is still TODO (`Server/game/src/Player.cpp`)
- NPC AI behavior is still TODO (`Server/game/src/NPC.cpp`)
- Several entity spawn fields in zone broadcasts still use placeholder values (class/level/type/item/count)
- Some packet IDs are still unhandled and currently log as unknown in server handlers

## Repository layout

```text
Server/
  CMakeLists.txt
  auth/
  chat/
  game/
  common/
client/
  Plugins/MMOClient/
```

## Tech stack

- C++17
- CMake
- Boost (Asio/Beast)
- OpenSSL
- LZ4
- MySQL client libraries
- Redis (hiredis)
- Unreal Engine plugin (client side)

## Prerequisites (Linux)

Install the core toolchain and dependencies:

```bash
sudo apt update
sudo apt install -y \
  build-essential cmake pkg-config \
  libboost-system-dev libboost-thread-dev \
  libssl-dev liblz4-dev libmysqlclient-dev libhiredis-dev
```

You also need running database services:

- MySQL (for accounts/characters/game state)
- Redis (for sessions/shard discovery/runtime state)

## Build servers

```bash
cd Server
cmake -S . -B build
cmake --build build --target auth_server chat_server game_server -j
```

## Configuration

Servers load key-value configs via `--config`.

Included examples:

- `Server/common/auth.cfg`
- `Server/common/game.cfg`

Typical keys:

- Network: `bind_ip`, `bind_port`
- MySQL: `mysql_host`, `mysql_port`, `mysql_user`, `mysql_password`, `mysql_db`
- Redis: `redis_host`, `redis_port`, `redis_user`, `redis_password`
- Logging: `log_level`, `log_to_file`, `log_file_dir`, `log_file_level`
- Web GUI: `webgui_enabled`, `webgui_port`, `webgui_allowed_ips`, `webgui_user`, `webgui_password`

## Run servers

From repository root:

```bash
./Server/build/auth_server --config ./Server/common/auth.cfg
./Server/build/chat_server --config ./Server/common/auth.cfg
./Server/build/game_server --config ./Server/common/game.cfg
```

Note: A dedicated `chat.cfg` is not currently shipped. You can create one or reuse `auth.cfg` with a unique `bind_port`.

## Unreal client plugin

Plugin path:

- `client/Plugins/MMOClient/MMOClient.uplugin`

The plugin exposes runtime networking functionality to connect gameplay code to backend services using the shared packet model.

## Development notes

- Packet contracts are centralized in `Server/common/include/Packets.h`.
- Shared server lifecycle/session behavior is in `Server/common/include/BaseServer.h`.
- Monitoring endpoints are in `Server/common/include/WebGuiServer.h`.
- Current implementation is a prototype foundation and should be hardened before internet-facing deployment.

## Community

- Discord (collaboration and support): https://discord.gg/SbNzjeZT

## Contributing

1. Fork the repository
2. Create a feature branch
3. Keep changes focused and well-tested
4. Open a pull request with clear context

## License

This project is licensed under the MIT License. See the `LICENSE` file for details.
