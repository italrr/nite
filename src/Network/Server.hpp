#ifndef GAME_SERVER_HPP
    #define GAME_SERVER_HPP

    #include "Network.hpp"

    namespace Game {

        struct SvClient {
            nite::IP_Port cl;
            String nickname;
            String clientId;
            UInt64 ping;
            nite::Packet lastPacket;
        };

        struct Server : Game::Net {
            Dict<String, Game::SvClient> clients;
            UInt8 maxClients;
            String serverId;
            bool init;
            Server();
            ~Server();
            void listen(UInt8 maxClients, UInt16 port);
            void step();
        };

    }

#endif 