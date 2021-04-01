#ifndef DP_SERVER_HPP
    #define DP_SERVER_HPP

    #include "Engine/Tools/Tools.hpp"
    #include "Engine/Network.hpp"
    #include "Network.hpp"

    struct Server : NetHandle {
        UInt8 genId;
        bool running;
        Server();
        bool listen(const nite::IP_Port &ip);
        void init(const Shared<GameState> &game);
        void clear();
        void update();
        void setState(UInt8 nstate); 
        void setState(UInt8 nstate, UInt8 who, Int32 stateArg); 
        void sendAll(nite::Packet &packet);     
    };

#endif