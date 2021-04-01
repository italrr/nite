#ifndef DP_CLIENT_HPP
    #define DP_CLIENT_HPP

    #include "Engine/Tools/Tools.hpp"
    #include "Engine/Network.hpp"
    #include "NEtwork.hpp"

    struct Client : NetHandle { 
        UInt32 svAck;
        UInt32 rcvOrder;
        UInt32 sentOrder;        
        UInt64 lastSentPing;
        UInt64 lastPing;        
        UInt8 id;
        UInt8 lastStateWho;
        Int32 lastStateArg;
        bool connected;
        unsigned money;
        nite::IP_Port sv;
        String nickname;
        Client();
        bool connect(const nite::IP_Port &ip);
        void init(const Shared<GameState> &game, const String &nickname);
        void clear();
        void update();
        void draw();
        void processIncomPackets();
        void deliverPacketQueue(); 
    };

#endif