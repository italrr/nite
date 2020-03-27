#ifndef GAME_NETWORK_HPP
    #define GAME_NETWORK_HPP
    
    #include "../Engine/Network.hpp" 
    #include "../Engine/Tools/Tools.hpp"

    namespace Game {
        
        enum NetState: UInt32 {
            Disconnected = 0,
            Connecting,
            Connected
        };      

        static const UInt32 RetryTimeOutInterval = 60;  
        static const UInt64 ClientTimeout = 1000 * 10; // TODO: inc this for release
        static const UInt64 ConnectingTimeout = 1000 * 3;
        static const UInt64 UpdatePhysicsTimeout = 32; // every 32 msecs a snapshot is sent
        static const UInt32 CLIENT_VERSION = 0x0; // TODO: This is temporary

       /*
            There are two types of packets. The ones that need to be ACK (svOrder from sender increase)
            and the ones that are to be received in order (lastOrder from sender is set)
       */

        /*
            Persisent deliveries are a way to guarantee this
            specific packet was delivered.
            It will retry according to the criteria (timeout and/or retries) until
            an ACK [ORDER] is received, after that packet is dropped.
        */

       /* 
            the uniqueId (uid) or called as 'netId' is basically:
            ip + port + sock
            this is decided by the server
       */


        struct PersisentDelivey {
            UInt64 retryInterval;
            int retries;
            int retry;
            nite::Packet packet;
            UInt32 order;
            UInt64 netId;
            nite::IP_Port cl;
            UInt64 lastRetry;
            UInt64 created;
            // call back mechanic for messages that depend on the delivery of others. chained-deliveries in other words
            std::function<void(nite::SmallPacket &payload, nite::IP_Port &cl)> onAck;
            nite::SmallPacket onAckPayload;
            PersisentDelivey(){
                this->onAck = [](nite::SmallPacket &payload, nite::IP_Port &cl){
                    
                };
            }
        };

        struct Net {
            Vector<Game::PersisentDelivey> deliveries;
            unsigned state;
            UInt64 lastState;
            bool init;
            nite::UDPSocket sock;
            Net();
            void setState(unsigned state);
            Game::PersisentDelivey& persSend(nite::IP_Port &client, nite::Packet &packet);
            Game::PersisentDelivey& persSend(nite::IP_Port &client, nite::Packet &packet, UInt64 retryInterval, int retries);  
            void dropPersFor(UInt64 netId);
            void dropPersForHeader(UInt64 netId, UInt16 header);
            void updateDeliveries();       
            void ack(nite::Packet &packet);
            void sendAck(nite::IP_Port &client, UInt32 ackId, UInt32 order);
            void bindOnAckFor(UInt16 header, std::function<void(nite::SmallPacket &payload, nite::IP_Port &cl)> lambda, nite::SmallPacket packet);
            void bindOnAckFor(UInt16 header, std::function<void(nite::SmallPacket &payload, nite::IP_Port &cl)> lambda);
            virtual void step();
        };




        /*
            default packet types

            ACK comment declares packets that need an ACK from the receiver.
            Every other packet flows using lastOrder.
        */
        enum PacketType : UInt16 {
            SV_ACK = 0,
            /*
                UINT32 ORDER
            */
            
            SV_PING,
            /*
            */
            
            SV_PONG,
            /*
            */                         
            
            SV_CONNECT_REQUEST,
            /*
                UINT32  CLIENTVERSION
                STRING  NICKNAME
            */
            
            SV_CONNECT_ACCEPT,  // ACK
            /*
                UINT64 CLIENTID 
                STRING SVNAME
            */  
            
            SV_CONNECT_REJECT,  
            /*
                STRING REASON
            */  
            
            SV_CLIENT_DROP,  
            /*
                STRING REASON (OPTIONAL)
            */    
            
            SV_CLIENT_DISCONNECT, 
            /*
                STRING REASON (OPTIONAL)
            */     
            
            SV_REMOTE_CMD_MSG, // ACK
            /*
                STRING MSG
                FLOAT R
                FLOAT G
                FLOAT B
            */    

            SV_REMOTE_CMD_EXEC,  // ACK
            /*
                STRING CMD
            */        
            
            SV_CLIENT_LIST,  // ACK (including yourself)
            /*
                UINT16 AMOUNT
                0: {
                    UINT64 UID(NETID)
                    UINT64 PING
                    STRING NICKNAME
                }
                ...
                n: {
                    UINT64 UID(NETID)
                    UINT64 PING
                    STRING NICKNAME
                }
            */                                                
            
            SV_NOTI_CLIENT_DROP,  // ACK
            /*
                UINT64 UID
                STRING REASON
            */           
            
            SV_BROADCAST_MESSAGE,  // ACK
            /*
                STRING MESSAGE
            */ 
            
            SV_CLIENT_JOIN,  // ACK
            /*
                UINT64 UID(NETID)
                STRING NICKNAME
            */
            
            SV_CHAT_MESSAGE,  // ACK
            /*
                UINT64 UID
                STRING MESSAGE
            */  
            
            SV_ENTITY_COUNT,  // ACK
            /*
                UINT16 AMOUNT
            */

            SV_CREATE_OBJECT,  // ACK
            /*
                UINT16 ID
                UINT16 SIGID
                FLOAT x
                FLOAT y
            */               

            SV_DESTROY_OBJECT,  // ACK
            /*
                UINT16 ID
            */
            
            SV_CREATE_BATCH_OBJECT, // ACK
            /*
                UINT16 AMOUNT
                0: {
                    UINT16 ID
                    UINT16 SIGID
                    FLOAT x
                    FLOAT y
                }
                ...
                n: {
                    UINT16 ID
                    UINT16 SIGID
                    FLOAT x
                    FLOAT y
                }                
            */    

            SV_DESTROY_BATCH_OBJECT, // ACK
            /*
                UINT16 AMOUNT
                0: UINT16 ID[0]
                ...
                n: UINT16 ID[n]             
            */ 

             SV_CLIENT_INPUT,
            /*
                UINT8 AMOUNT
                0: {
                    UINT8 KEY
                    UINT8 TYPE
                    UINT8 NEXT_KEY_STROKE
                }
                ...
                n: {
                    UINT8 KEY
                    UINT8 TYPE
                    UINT8 NEXT_KEY_STROKE
                }
            */    

            SV_UPDATE_PHYSICS_OBJECT,
            /*
                UINT16 AMOUNT
                0: {
                    UINT16 ID
                    FLOAT X
                    FLOAT Y
                    FLOAT SX // SPEED
                    FLOAT SY
                }
                ...
                n: {
                    UINT16 ID
                    FLOAT X
                    FLOAT Y
                    FLOAT SX
                    FLOAT SY                    
                }                
            */ 

            SV_UPDATE_WORLD_SIMULATION_PROPS, // ACK
            /*
                FLOAT TIMESCALE
            */  

            SV_UPDATE_OBJECT_RELATIVE_TIMESCALE, // ACK
            /*
                UINT16 ID
                FLOAT TIMESCALE
            */    
            SV_RCON, // ACK
            /*
                STRING PASSWORD(hash)
            */   
            SV_SET_GAME_START, // ACK
            /*
            */  
            SV_NOTI_ENTITY_OWNER, // ACK
            /*
                UINT16 ENTID
            */                                                      
            SV_SET_ENTITY_SKILLS, // ACK
            /*
                UINT16 ENTID
                UINT8 AMOUNT
                0: {
                    UINT16 SKILLID[0]
                    UINT8 LV[0]
                }
                ...
                n: {
                    UINT16 SKILLID[n]
                    UINT8 LV[n]
                }                

            */ 
            SV_ADD_ENTITY_SKILL, // ACK
            /*
                UINT16 ENTID
                UINT16 SKILLID
                UINT8 LV           

            */             
            SV_REMOVE_ENTITY_SKILLS, // ACK
            /*
                UINT16 ENTID
                UINT8 AMOUNT
                0: {
                    UINT16 SKILLID[0]
                }
                ...
                n: {
                    UINT16 SKILLID[n]
                }                

            */  
            SV_REMOVE_ENTITY_SKILL, // ACK
            /*
                UINT16 ENTID
                UINT16 SKILLID     

            */               
            SV_SET_ENTITY_ACTIONABLES, // ACK
            /*
                UINT16 ENTID
                UINT8 AMNT
                0: {
                    UINT8 TYPE[0]
                    UINT16 ID[0]
                }
                ...
                n: {
                    UINT8 TYPE[n]
                    UINT16 ID[n]
                }                

            */  
            SV_ADD_EFFECT, // ACK
            /*
                UINT16 ENTID
                UINT16 TYPE (effectId)
                UINT16 INSID
                {
                    CUSTOM PAYLOAD // Ideally less than 32 bytes
                }

            */
            SV_REMOVE_EFFECT, // ACK     
            /*
                UINT16 ENTID
                UINT16 INSID
            */    
            SV_UPDATE_EFFECT, // ACK     
            /*
                UINT16 ENTID
                UINT16 INSID
                {
                    CUSTOM PAYLOAD
                }
            */  
            SV_ADD_ITEM,
            /*
                UINT16 ENTID
                UINT16 ITEMID
                UINT16 SLOTID
                UINT16 AMNT
            */
            SV_REMOVE_ITEM,
            /*
                UINT16 ENTID
                UINT16 ITEMID
                UINT16 SLOTID
                UINT16 AMNT
            */ 
            SV_UPDATE_ENTITY_ALL_STAT,
            /*
                UINT16 ENTID
                {
                    sizeof(HealthStat)
                    sizeof(BaseStat)
                    sizeof(ComplexCStat)
                }
            */           
            SV_UPDATE_ENTITY_BASE_STAT,
            /*
                UINT16 ENTID
                {
                    sizeof(BaseStat)
                }
            */
            SV_UPDATE_ENTITY_HEALTH_STAT,
            /*
                UINT16 ENTID
                {
                    sizeof(HealthStat)
                }
            */                                           
            SV_UPDATE_ENTITY_COMPLEX_STAT,
            /*
                UINT16 ENTID
                {
                    sizeof(ComplexStat)
                }
            */ 
            SV_NOTIFY_ENTITY_DEATH,
            /*
                UINT16 ENTID
                {
                    sizeof(HealthStat)
                }
            */  
            SV_NOTIFY_ENTITY_REVIVE,
            /*
                UINT16 ENTID
                {
                    sizeof(HealthStat)
                }
            */ 
           SV_SET_GAME_OVER,   
           SV_SET_GAME_RESTART,                 

        };


    }


#endif 