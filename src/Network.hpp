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
        Dict<unsigned, OwnedProperty> owns;
        UInt32 money;
        nite::Color color;
        UInt64 lastSentPing;
        UInt64 lastPing;   
        UInt32 svAck;    // id for packets that require an ACK from client
        UInt32 lastRecvOrder; 
        UInt32 lastSentOrder;     
        UInt8 currentSpace;    
        void addProperty(int id);
        void removeProperty(int id);
        void addUpgrade(unsigned type, int id);
        void removeUpgrade(unsigned type, int id);  
        Vector<UInt8> route; 
        ClientProxy(){
            lastRecvOrder = 0;
            lastSentOrder = 1;
            svAck = 0;   
            currentSpace = 0;         
            lastSentPing = nite::getTicks();
            lastPing = nite::getTicks();            
        }     
        void setRoute(UInt8 target, const Vector<UInt8> &route){
            if(this->route.size() > 0){
                currentSpace = target;
            }
            this->route = route;
        }
    };

    struct BindAckCallback {
        Lambda<void(const UInt8 id, nite::Packet &packet)> callback;
        UInt8 clientId;
        UInt16 headerId;
        bool disposable;
        UInt16 id;
        BindAckCallback(){
            this->disposable = false;
        }
        BindAckCallback(const Lambda<void(const UInt8 id, nite::Packet &packet)> &callback, UInt8 clientId, UInt16 headerId){
            this->callback = callback;
            this->clientId = clientId;
            this->headerId = headerId;
            this->disposable = false;
        }
    };

    struct PersisentDelivey {
        nite::Packet packet;
        UInt32 ack;
        UInt16 header;
        nite::IP_Port ip;
        UInt64 lastRetry;
        std::function<void(nite::SmallPacket &payload, nite::IP_Port &cl)> onAck;
        nite::SmallPacket onAckPayload;
        PersisentDelivey(){
            this->onAck = [](nite::SmallPacket &payload, nite::IP_Port &cl){

            };
        }
    };    

    struct NetHandle {

        Vector<nite::Packet> packetQueue;
        Vector<nite::Packet> rcvPackets;
        void sendPacketFor(const nite::IP_Port &ip, nite::Packet &packet);
        void sendPacketForMany(const Vector<nite::IP_Port> &ips, nite::Packet &packet);
        void sendPersPacketFor(const nite::IP_Port &ip, nite::Packet &packet, UInt32 ack);
        void sendPersPacketForMany(const Vector<nite::IP_Port> &ips, nite::Packet &packet, const Vector<UInt32> acks);

        void dropPersFor(const nite::IP_Port &ip);
        void dropPersForHeader(const nite::IP_Port &ip, UInt16 header);
        void ack(nite::Packet &packet);
        void sendAck(nite::IP_Port &ip, UInt32 ackId);        
        void updateDeliveries();
        Vector<Shared<PersisentDelivey>> deliveries;
        nite::UDPSocket socket;
        bool isServer;

        void bindOnAckFor(UInt32 ack, std::function<void(nite::SmallPacket &payload, nite::IP_Port &ip)> lambda, nite::SmallPacket packet);
        void bindOnAckFor(UInt32 ack, std::function<void(nite::SmallPacket &payload, nite::IP_Port &ip)> lambda);

        void bindOnAckForHeader(UInt16 header, std::function<void(nite::SmallPacket &payload, nite::IP_Port &ip)> lambda, nite::SmallPacket packet);
        void bindOnAckForHeader(UInt16 header, std::function<void(nite::SmallPacket &payload, nite::IP_Port &ip)> lambda);
        
                
        Shared<ClientProxy> getClientByIp(const nite::IP_Port &ip);
        Vector<nite::IP_Port> getAllClientsIps();
        Vector<UInt32> getAllClientsAcks();

        Vector<Lambda<void(UInt8 nstate, UInt8 who, Int32 arg)>> onChangeCallback;
        Dict<unsigned, BindAckCallback> onReceiveCallback;
        Shared<GameState> game;
        UInt64 lastState;
        UInt8 state;
        std::map<UInt8, Shared<ClientProxy>> clients; 

        NetHandle(){
            lastBAckCallbackId = 0;
            isServer = false;
        }
        void setState(UInt8 nstate){
            setState(nstate, 0, 0);
        }
        unsigned lastBAckCallbackId;
        void addBindAckCallback(const BindAckCallback &cb){
            auto copy = cb;
            copy.id = ++lastBAckCallbackId;
            onReceiveCallback[copy.id] = copy;
        }
        void onAck(UInt8 id, nite::Packet &packet){
            Vector<int> dispose;
            for(auto &it : onReceiveCallback){
                if((it.second.clientId == id || it.second.clientId == 0) && packet.getHeader() == it.second.headerId){
                    it.second.callback(id, packet);
                    if(it.second.disposable){
                        dispose.push_back(it.second.id);
                    }
                }
            }
            for(int i = 0; i < dispose.size(); ++i){
                onReceiveCallback.erase(dispose[id]);
            }
        }
        void setState(UInt8 nstate, UInt8 who, Int32 stateArg){
            this->lastState = nite::getTicks();
            for(int i = 0; i < onChangeCallback.size(); ++i){                
                onChangeCallback[i](nstate, who, stateArg);
            }
        }
    };

    enum SERVER_STATE : UInt8 {
        ST_WAITING_FOR_PLAYERS,
        ST_PRE_GAME,
        ST_CHOOSE_ORDERS, // throw dice to choose order
        ST_PLAYER_THROW_DICE,
        ST_GAME_START,
        ST_PLAYER_PRETURN,
        ST_PLAYER_TURN,
    };



    enum DP_PACKET : UInt16 {
        SV_MULTI_PART_PACKET,
        SV_CONNECT,
        /*
            STRING  NICKNAME
            STRING  COLOR
        */
        SV_CONNECT_ACCEPT,
        /*
            STRING  NICKNAME
            UINT8   ID
            STRING  GAMETYPE
            UINT8   PLAYER_N
            {
                STRING  NICKNAME
                STRING  COLOR
                UINT8   ID
                UINT32  MONEY
            } .. PLAYER_N
        */
        SV_CONNECT_REJECT,
        /*
            STRING  REASON
        */          
        SV_CLIENT_JOIN,
        /*
            STRING  NICKNAME
            STRING  COLOR
            UINT8   ID
            UINT32  MONEY
        */
        SV_ACK,
        SV_PING,
        SV_PONG,
        SV_SHUFFLE_DICE,
        /*
            INT8    TRUE/FALSE
        */     
        SV_GAME_STATE,
        /*
            UINT8   CURRENT_STATE
            UINT8   WHO
            INT32   STATE_ARG
        */
        SV_ADVANCE_PLAYER_TO,
        /*
            UINT8   ID
            UINT8   TARGET
            UINT8   STEP_N
            {
                UINT8   SPACE
            } .. STEP_N
        */
        SV_BROADCAST_MESSAGE,
        /*
            STRING  MESSAGE
            STRING  COLOR   
        */

    };

#endif