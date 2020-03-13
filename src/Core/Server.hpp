#ifndef GAME_SERVER_HPP
    #define GAME_SERVER_HPP

    #include "Network.hpp"
    #include "Object.hpp"
    #include "World.hpp"
    #include "../RING/Map.hpp"
    #include "Input.hpp"
    #include "../Entity/Effect.hpp"
    #include "../Entity/Inventory.hpp"
    
    namespace Game {

        namespace SvClientRole {
            enum SvClientRole : UInt8 {
                Player = 0,
                Mod,
                Admin
            };
        }

        struct SvClient {
            nite::IP_Port cl;
            Game::InputSimulator input;
            String nickname;
            UInt64 clientId;
            UInt16 entityId;
            UInt64 ping;
            UInt8 role;
            nite::Packet lastPacket;
            UInt32 svOrder;    // id for packets that require an ACK from client
            UInt32 lastRecvOrder; 
            UInt32 lastSentOrder;  
            UInt64 lastPing;    
            UInt64 lastPacketTimeout;
            SvClient(){
                role = Game::SvClientRole::Player;
                lastRecvOrder = 0;
                lastSentOrder = 1;
                svOrder = 0;
                entityId = 0;
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
            UInt64 physicsUpdate;
            UInt64 lastPlayerInfoSent;
            Dict<UInt64, UInt16> players;
            Vector<Shared<Game::RING::Map>> maps;
            Server();
            ~Server();
            void removeClient(UInt64 uid);
            void dropClient(UInt64 uid);
            void dropClient(UInt64 uid, String reason);
            Game::SvClient *getClient(const String &nickname);
            Game::SvClient *getClient(UInt64 uid);
            Game::SvClient *getClientByIp(nite::IP_Port &ip);
            Game::SvClient *getClientByEntityId(UInt16 entityId);
            void persSendAll(nite::Packet packet, UInt64 timeout, int retries);
            void sendAll(nite::Packet packet);
            void broadcast(const String &message);
            void sendRemoteCmdMsg(UInt64 uid, const String &msg, const nite::Color &color);
            void preinit();
            void listen(const String &name, UInt8 maxClients, UInt16 port);
            void update();
            void close();
            void clear();
            void game();


            /*
                game
            */

            // add/remove
            void addItem(UInt16 entityId, Shared<Game::BaseItem> item);
            void removeItem(UInt16 entityId, UInt16 itemId, UInt16 amnt);            
            void addEffect(UInt16 entityId, Shared<Game::Effect> &eff);
            void removeEffect(UInt16 entityId, UInt16 insId);
            void addSkill(UInt16 entityId, UInt16 skillId, UInt8 lv);
            void removeSkill(UInt16 entityId, UInt16 skillId);
            
            // send info
            void sendSkillList(UInt64 uid, UInt16 entityId);
            void sendPlayerList(UInt64 uid); // sends it everyone
            void notifyAddSkill(UInt64 uid, UInt16 skillId, UInt8 lv);
            void notifyRemoveSkill(UInt64 uid, UInt16 skillId);
            void notifyAddEffect(UInt64 uid, UInt16 type, UInt16 insId);
            void notifyRemoveEffect(UInt64 uid, UInt16 insId);
            void notifyUpdateEffect(UInt64 uid, UInt16 insId);
            
            // core
            Shared<Game::NetObject> spawn(Shared<Game::NetObject> obj);
            bool destroy(UInt32 id);            
            void setupGame(const String &name, int maxClients, int maps);
            Shared<Game::NetObject> createPlayer(UInt64 uid, UInt32 lv);
            bool removePlayer(UInt64 uid);
            bool killPlayer(UInt64 uid);
        };

    }

#endif 