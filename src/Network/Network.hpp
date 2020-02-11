#ifndef GAME_NETWORK_HPP
    #define GAME_NETWORK_HPP
    
    #include "../Engine/Network.hpp" 

    namespace Game {
        
        enum NetState: UInt32 {
            Disconnected = 0,
            Connecting,
            Connected
        };      

        static const UInt32 RetryTimeOutInterval = 60; 
        static const UInt64 ClientTimeout = 1000 * 10;
        static const UInt64 ConnectingTimeout = 3000;
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
        };

        struct Net {
            Vector<Game::PersisentDelivey> deliveries;
            unsigned state;
            UInt64 lastState;
            bool init;
            nite::UDPSocket sock;
            Net();
            void setState(unsigned state);
            void persSend(nite::IP_Port &client, nite::Packet &packet);
            void persSend(nite::IP_Port &client, nite::Packet &packet, UInt64 retryInterval, int retries);  
            void dropPersFor(UInt64 netId);
            void updateDeliveries();       
            void ack(nite::Packet &packet);
            void sendAck(nite::IP_Port &client, UInt32 ackId, UInt32 order);
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

        };


    }


#endif 