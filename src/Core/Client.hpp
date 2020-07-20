#ifndef GAME_CLIENT_HPP
    #define GAME_CLIENT_HPP
    
    #include "../Engine/Map.hpp"

    #include "Network.hpp"   
    #include "Object.hpp"
    #include "Input.hpp"
    #include "World.hpp"
    #include "Camera.hpp"
    #include "../UI/HUD.hpp"
    #include "../UI/Icon.hpp"
    #include "../UI/IngameMenu.hpp"

    namespace Game {

        struct ClClient {
            UInt64 uid;
            UInt64 ping;
            String nickname;
            ClClient(){

            }
            ClClient(UInt64 uid, String nickname){
                this->uid = uid;
                this->nickname = nickname;
            }
        };

        struct Client : Game::Net {
            Game::IconManager icons;
            Game::Input input;
            Game::HUD hud;
            Game::Camera camera;
            Game::InGameMenu igmenu;
            Dict<UInt64, Game::ClClient> clients;
            Game::NetWorld world;
            Shared<nite::Map> map;
            nite::IP_Port sv;
            UInt64 clientId;
            UInt64 serverId;
            UInt16 entityId;
            String serverName;
            String nickname;
            UInt64 ping;
            nite::Packet lastPacket;
            UInt64 lastPacketTimeout;
            UInt64 lastPing;
            UInt32 svOrder;
            UInt32 rcvOrder;
            UInt32 sentOrder;
            Vector<Game::NetObject*> localMasks;
            bool connected;
            Client();
            ~Client();
            void setCurrentMap(Shared<nite::Map> &map);
            Game::EntityBase *getEntity(UInt16 id);
            void disconnect(const String &reason);
            void disconnect();
            void connect(const String &ip, UInt16 port);
            void connect();
            void setup(const String &nickname);
            void onStart();
            void update();
            void game();
            void render();
            void clear();
            void sendChatMsg(const String &msg);
        };


    }


#endif 