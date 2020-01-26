#ifndef GAME_SERVER_HPP
    #define GAME_SERVER_HPP

    #include "Network.hpp"

    namespace Game {

        namespace SvClientRole {
            static const unsigned Player = 0;
            static const unsigned Mod = 1;
            static const unsigned Admin = 2;
        }

        struct SvClient {
            nite::IP_Port cl;
            String nickname;
            UInt64 clientId;
            UInt64 ping;
            unsigned role;
            nite::Packet lastPacket;
            UInt32 svOrder;    // id for packets that require an ACK from client
            UInt32 lastOrder;  // packets with order less than this are dropped
            UInt64 lastPing;    
            UInt64 lastPacketTimeout;
            SvClient(){
                svOrder = 0;
            }
        };

        struct Server : Game::Net {
            Dict<UInt64, Game::SvClient> clients;
            UInt8 maxClients;
            Server();
            ~Server();
            void removeClient(UInt64 uid);
            void dropClient(UInt64 uid);
            void dropClient(UInt64 uid, String reason);
            Game::SvClient *getClient(const String &nickname);
            Game::SvClient *getClient(UInt64 uid);
            void listen(UInt8 maxClients, UInt16 port);
            void step();
        };

    }

#endif 