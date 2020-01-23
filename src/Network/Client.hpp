#ifndef GAME_CLIENT_HPP
    #define GAME_CLIENT_HPP
    
    #include "Network.hpp"   

    namespace Game {

        struct Client : Game::Net {
            nite::IP_Port sv;
            String clientId;
            String serverId;
            String nickname;
            UInt64 ping;
            bool init;
            Client();
            ~Client();
            void connect(const String &ip, UInt16 port);
            void connect();
            void setup(const String &nickname);
            void step();
        };


    }


#endif 