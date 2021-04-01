#ifndef DP_CLIENT_HPP
    #define DP_CLIENT_HPP

    #include "Engine/Tools/Tools.hpp"
    #include "Engine/Network.hpp"
    #include "NEtwork.hpp"

    struct Client : NetHandle { 
        UInt64 lastSentPing;
        UInt64 lastPing;        
        UInt8 id;
        UInt8 lastStateWho;
        Int32 lastStateArg;
        bool connected;
        unsigned money;
        String nickname;
        Client();
        bool connect(const nite::IP_Port &ip);
        void init(const Shared<GameState> &game, const String &nickname);
        void clear();
        void update();
        void draw();
    };

#endif