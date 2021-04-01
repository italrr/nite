#ifndef DP_PACKETS_HPP
    #define DP_PACKETS_HPP
    
    #include "Engine/Types.hpp"
    #include "Engine/Network.hpp"
    #include "GameType.hpp"
    #include "State.hpp"

    static const UInt16 DEFAULT_PORT = 12949;
    
    enum UpgradeType : unsigned {
        HOUSE,
        HOTEL
    };

    struct OwnedProperty {
        unsigned id;
        unsigned houses;
        unsigned hotels;
    };

    struct ClientProxy {
        UInt8 id;
        UInt8 order;
        String nickname;
        nite::IP_Port ip;
        Int32 socket;
        Dict<unsigned, OwnedProperty> owns;
        UInt32 money;
        UInt64 lastSentPing;
        UInt64 lastPing;   
        void addProperty(int id);
        void removeProperty(int id);
        void addUpgrade(unsigned type, int id);
        void removeUpgrade(unsigned type, int id);        
        ClientProxy(){
            lastSentPing = nite::getTicks();
            lastPing = nite::getTicks();            
        }     
    };

    struct NetHandle {
        Vector<Lambda<void(UInt8 nstate)>> onChangeCallback;
        Shared<GameState> game;
        nite::TCPSocket sock;
        UInt64 lastState;
        UInt8 state;
        Dict<UInt8, Shared<ClientProxy>> clients; 
        Shared<ClientProxy> getClientBySock(Int32 id);
        void setState(UInt8 nstate){
            this->lastState = nite::getTicks();
            this->state = nstate;
            for(int i = 0; i < onChangeCallback.size(); ++i){                
                onChangeCallback[i](nstate);
            }
        }
    };

    enum SERVER_STATE : UInt8 {
        ST_WAITING_FOR_PLAYERS,
        ST_GAME_START,
        ST_CHOOSE_ORDERS // throw dice to choose order
    };

    enum DP_PACKET : UInt16 {
        SV_PING,
        SV_PONG,
        SV_JOIN,
        /*
            STRING  NICKNAME
        */
        SV_ACCEPT,
        /*
            STRING  NICKNAME
            UINT8   ID
            STRING  GAMETYPE
            UINT8   PLAYER_N
            {
                STRING  NICKNAME
                UINT16  ID
                UINT32  MONEY
            } .. PLAYER_N
        */
        SV_REJECT,
        /*
            STRING  REASON
        */       
        SV_GAME_STATE,
        /*
            UINT8   CURRENT_STATE
            UINT8   WHO
            INT32   STATE_ARG
        */

    };

#endif