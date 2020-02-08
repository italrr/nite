#ifndef GAME_SERVER_HPP
    #define GAME_SERVER_HPP

    #include "Network.hpp"
    #include "Object.hpp"
    #include "World.hpp"
    #include "../RING/Map.hpp"
    
    namespace Game {

        enum SvClientRole : UInt8 {
            Player = 0,
            Mod,
            Admin
        };

        struct SvClient {
            nite::IP_Port cl;
            String nickname;
            UInt64 clientId;
            UInt64 ping;
            UInt8 role;
            nite::Packet lastPacket;
            UInt32 svOrder;    // id for packets that require an ACK from client
            UInt32 lastRecvOrder; 
            UInt32 lastSentOrder;  
            UInt64 lastPing;    
            UInt64 lastPacketTimeout;
            SvClient(){
                lastRecvOrder = 0;
                lastSentOrder = 1;
                svOrder = 0;
            }
        };

        struct SvTilesetSource {
            String path;
            String hash;
            size_t size;
        };

        struct Server : Game::Net {
            String name;
            Dict<String, Game::SvTilesetSource> tilesets;
            Game::NetWorld world;
            Dict<UInt64, Game::SvClient> clients;
            UInt8 maxClients;
            UInt64 lastPlayerInfoSent;
            Server();
            ~Server();
            void removeClient(UInt64 uid);
            void dropClient(UInt64 uid);
            void dropClient(UInt64 uid, String reason);
            Game::SvClient *getClient(const String &nickname);
            Game::SvClient *getClient(UInt64 uid);
            void persSendAll(nite::Packet packet, UInt64 timeout, int retries);
            void sendAll(nite::Packet packet);
            void broadcast(const String &message);
            void preinit();
            void listen(const String &name, UInt8 maxClients, UInt16 port);
            void update();
            void close();
            void clear();
            void game();
            // game
            void sendInfoPlayerList(UInt64 uid);
            Vector<UInt64> players;
            Vector<Shared<Game::RING::Map>> maps;
            void setupGame(const String &name, int maxClients, int maps);
            UInt16 spawn(Shared<Game::NetObject> obj);
            void destroy(UInt32 id);
            void destroy(Shared<Game::NetObject> obj);
            Shared<Game::NetObject> createPlayer(Game::SvClient &cl, UInt32 lv);
            Shared<Game::NetObject> createPlayer(UInt64 uid, UInt32 lv);
            void removePlayer(UInt64 uid);
            void removePlayer(Game::SvClient &cl);
            void killPlayer(UInt64 uid);
            void killPlayer(Game::SvClient &cl);
        };

    }

#endif 