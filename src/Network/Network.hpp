#ifndef GAME_NETWORK_HPP
    #define GAME_NETWORK_HPP
    
    #include "../Engine/Network.hpp" 

    namespace Game {
        
        namespace NetState {
            static const unsigned Disconnected = 0;
            static const unsigned Connecting = 1;
            static const unsigned Connected = 2;
        }        
        static const UInt64 ConnectingTimeout = 2000; 
        static const UInt32 CLIENT_VERSION = 0x0; // TODO: This is temporary

        struct Net {
            unsigned state;
            UInt64 lastState;
            nite::UDPSocket sock;
            Net();
            void setState(unsigned state);
            String generateId();
            virtual void step();
        };

        // default packet types
        namespace PacketType {
            static const UInt16 SV_ACK                  = 0;
            /*
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
                STRING  CLIENTID
                STRING  NICK
            */
            static const UInt16 SV_CONNECT_ACCEPT       = 4;  
            /*
                STRING SERVERID 
            */  
            static const UInt16 SV_CONNECT_REJECT       = 5;  
            /*
                STRING REASON
            */            

        }


    }


#endif 