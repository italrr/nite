#ifndef GAME_CLIENT_HPP
    #define GAME_CLIENT_HPP
    
    #include "Network.hpp"   

    namespace Game {

        struct Client : Game::Net {
            nite::IP_Port sv;
            UInt64 clientId;
            UInt64 serverId;
            String nickname;
            UInt64 ping;
            nite::Packet lastPacket;
            UInt64 lastPacketTimeout;
            UInt64 lastPing;
            UInt32 svOrder;
            UInt32 lastOrder;
            bool connected;
            Client();
            ~Client();
            void disconnect(const String &reason);
            void disconnect();
            void connect(const String &ip, UInt16 port);
            void connect();
            void setup(const String &nickname);
            void step();
            void clear();
        };


    }


#endif 