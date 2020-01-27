#ifndef GAME_NETWORK_HPP
    #define GAME_NETWORK_HPP
    
    #include "../Engine/Network.hpp" 

    namespace Game {
        
        namespace NetState {
            static const unsigned Disconnected = 0;
            static const unsigned Connecting = 1;
            static const unsigned Connected = 2;
        }        

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


        struct PersisentDelivey {
            UInt64 retryInterval;
            UInt64 timeout;
            int retries;
            int retry;
            nite::Packet packet;
            UInt32 order;
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
            void persSend(nite::IP_Port &client, nite::Packet &packet, UInt64 timeout, int retries);  
            void updateDeliveries();       
            void ack(nite::Packet &packet);
            void sendAck(nite::IP_Port &client, UInt32 order);
            virtual void step();
        };

        /*
            default packet types

            ACK comment declares packets that need an ACK from the receiver.
            Every other packet flows using lastOrder.
        */
        namespace PacketType {
            static const UInt16 SV_ACK                  = 0;
            /*
                UINT32 ORDER
            */
            static const UInt16 SV_PING                 = 1;
            /*
            */
            static const UInt16 SV_PONG                 = 2;
            /*
            */                         
            static const UInt16 SV_CONNECT_REQUEST      = 3;
            /*
                UINT32  CLIENTVERSION
                STRING  NICKNAME
            */
            static const UInt16 SV_CONNECT_ACCEPT       = 4;  // ACK
            /*
                UINT64 CLIENTID 
            */  
            static const UInt16 SV_CONNECT_REJECT       = 5;  
            /*
                STRING REASON
            */  
            static const UInt16 SV_CLIENT_DROP          = 6;  
            /*
                STRING REASON (OPTIONAL)
            */    
            static const UInt16 SV_CLIENT_DISCONNECT    = 7; 
            /*
                STRING REASON (OPTIONAL)
            */     
            static const UInt16 SV_REMOTE_CMD_EXEC      = 8;  // ACK
            /*
                STRING CMD
            */            
                                         

        }


    }


#endif 