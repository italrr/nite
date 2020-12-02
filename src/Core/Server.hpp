#ifndef GAME_SERVER_HPP
    #define GAME_SERVER_HPP

    #include "Network.hpp"
    #include "Object.hpp"
    #include "Input.hpp"
    #include "../Entity/Effect.hpp"
    #include "../Entity/Inventory.hpp"

    #include "../RING/RING.hpp"
    
    namespace Game {

        namespace SvClientRole {
            enum SvClientRole : UInt8 {
                Player = 0,
                Mod,
                Admin
            };
        }

        enum ServerState: UInt32 {
            Off = 0,
            Idle,
            Waiting,      // waiting for players to load the game (may include download the map from server)
            InGame,
            GameOver,
            ShuttingDown
        };        

        struct SvClient {
            nite::IP_Port cl;
            String nickname;
            UInt64 clientId;
            UInt16 entityId;
            UInt64 ping;
            UInt8 role;
            nite::Packet lastPacket;
            UInt32 svAck;    // id for packets that require an ACK from client
            UInt32 lastRecvOrder; 
            UInt32 lastSentOrder;  
            UInt64 lastPing;    
            UInt64 lastPacketTimeout;
            UInt64 lastState;
            bool ready;
            SvClient(){
                role = Game::SvClientRole::Player;
                lastRecvOrder = 0;
                lastSentOrder = 1;
                svAck = 0;
                entityId = 0;
                ready = false;
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
            Dict<UInt64, Game::SvClient> clients;
            UInt8 maxClients;
            UInt64 lastPlayerInfoSent;
            Dict<UInt64, UInt16> players;
            Vector<Shared<nite::Map>> maps;
            Server();
            ~Server();
            void setState(unsigned state);
            void removeClient(UInt64 uid);
            void dropClient(UInt64 uid);
            void dropClient(UInt64 uid, String reason);
            Game::SvClient *getClient(const String &nickname);
            Game::SvClient *getClient(UInt64 uid);
            Game::SvClient *getClientByIp(nite::IP_Port &ip);
            Game::SvClient *getClientByEntityId(UInt16 entityId);
            Game::EntityBase *getEntity(UInt16 id);
            void persSendAll(nite::Packet &packet, UInt64 timeout, int retries);
            void sendAll(nite::Packet &packet);
            void broadcast(const String &message);
            void sendRemoteCmdMsg(UInt64 uid, const String &msg, const nite::Color &color);
            void preinit();
            void listen(const String &name, UInt8 maxClients, UInt16 port);
            void update();
            void close();
            void start();
            void restart();
            void clear();
            void game();


            /*
                game
            */

            // add/remove
            void addItem(UInt16 entityId, Shared<Game::ItemBase> item);
            void removeItem(UInt16 entityId, UInt16 slotId, UInt16 amnt);            
            void addEffect(UInt16 entityId, Shared<Game::Effect> &eff);
            void removeEffect(UInt16 entityId, UInt16 insId);
            void addSkill(UInt16 entityId, UInt16 skillId, UInt8 lv);
            void removeSkill(UInt16 entityId, UInt16 skillId);
            
            // send info / notify
            void sendSkillList(UInt64 uid, UInt16 entityId);
            void sendPlayerList(UInt64 uid); // sends it everyone
            void notifyAddSkill(UInt64 uid, UInt16 skillId, UInt8 lv);
            void notifyRemoveSkill(UInt64 uid, UInt16 skillId);
            void notifyAddEffect(UInt64 uid, UInt16 type, UInt16 insId);
            void notifyRemoveEffect(UInt64 uid, UInt16 insId);
            void notifyUpdateEffect(UInt64 uid, UInt16 insId);
            void notifyAddItem(UInt64 uid, UInt16 itemId, UInt16 slotId, UInt16 qty);
            void notifyRemoveItem(UInt64 uid, UInt16 itemId, UInt16 slotId, UInt16 qty); 
            void notifyDeath(UInt64 uid);
            void notifyGameOver(UInt64 uid);
            void notifyGameRestart(UInt64 uid);

            void notifyUpdateEquipSlots(UInt64 uid);

            void notifyUpdateInvList(UInt64 uid);       
            
            // core
            Shared<Game::NetObject> spawn(Shared<Game::NetObject> obj);
            bool destroy(UInt16 id);            
            void setupGame(const String &name, int maxClients, int maps);
            Shared<Game::NetObject> createPlayer(UInt64 uid, UInt32 lv, float x, float y);
            Shared<Game::NetObject> createMob(UInt16 sig, UInt32 lv, float x, float y);
            bool removePlayer(UInt64 uid);
            bool killPlayer(UInt64 uid);
            void createPlayersOnStart(UInt16 initialHeader);
        };

    }

#endif 