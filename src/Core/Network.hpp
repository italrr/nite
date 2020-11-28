#ifndef GAME_NETWORK_HPP
    #define GAME_NETWORK_HPP

    #include <stdio.h>
    #include "../Engine/Network.hpp"
    #include "../Engine/Tools/Tools.hpp"
    #include "../Engine/Map.hpp"
    #include "../Entity/Traps/Trap.hpp"
    #include "../Entity/Vfx/Vfx.hpp"
    #include "World.hpp"

    namespace Game {

        enum NetState: UInt32 {
            Disconnected = 0,
            Connecting,
            Connected
        };

        static const UInt32 RetryTimeOutInterval = 60;
        static const UInt64 ClientTimeout = 1000 * 30; // TODO: inc this for release
        static const UInt64 ConnectingTimeout = 1000 * 3;
        static const UInt64 UpdatePhysicsTimeout = 32; // every 32 msecs a snapshot is sent
        static const float ClientPositionInterp = 0.18f;
        static const float ClientSpeedInterp = 0.18f;
        static const nite::Vec2 ClientRepositionThreshold = nite::Vec2(32.0f);
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
            bool stale;
            void markStale(){
                if(stale){
                    return;
                }
                stale = true;
                lastStaleTick = nite::getTicks();
            }
            UInt64 lastStaleTick;
            // call back mechanic for messages that depend on the delivery of others. chained-deliveries in other words
            std::function<void(nite::SmallPacket &payload, nite::IP_Port &cl)> onAck;
            nite::SmallPacket onAckPayload;
            PersisentDelivey(){
                stale = false;
                lastStaleTick = nite::getTicks();
                created = nite::getTicks();
                this->onAck = [](nite::SmallPacket &payload, nite::IP_Port &cl){

                };
            }
        };


        struct RemoteClock {
            UInt64 time;
            UInt64 lastTick;
            UInt64 delta;
            void update();
            void set(UInt64 time);
            RemoteClock(){
                this->time = nite::getTicks();
            }
        };

        struct Net {
            bool isServer;
            Game::RemoteClock clock;
            Vector<Shared<Game::PersisentDelivey>> deliveries;
            unsigned state;
            UInt64 lastState;
            Game::NetWorld world;
            bool init;
            Game::TrapDevice traps;
            UInt16 lastInitTfId;
            nite::UDPSocket sock;
            Game::VfxDevice vfx;
            Shared<nite::Map> map;
            nite::FileTransfer::UDPClient ft;
            void setCurrentMap(Shared<nite::Map> &map);
            Net();
            void setState(unsigned state);
            Shared<Game::PersisentDelivey> persSend(nite::IP_Port &client, nite::Packet &packet);
            Shared<Game::PersisentDelivey> persSend(nite::IP_Port &client, nite::Packet &packet, UInt64 retryInterval, int retries);
            void updateDeliveries();
            void dropPersFor(UInt64 netId);
            void dropPersForHeader(UInt64 netId, UInt16 header);
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
                SEVER-SIDE ONY:
                UINT64 REMOTE_TIME
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

            SV_AWAIT_CLIENT_LOAD,  // ACK
            /*
                STRING MAPHASH
            */

            SV_CLIENT_LOAD_READY, // ACK
            /*
                STRING MAPHASH
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

            SV_CREATE_OBJECT_BATCH, // ACK
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

            SV_DESTROY_OBJECT_BATCH, // ACK
            /*
                UINT16 AMOUNT
                0: UINT16 ID[0]
                ...
                n: UINT16 ID[n]
            */

             SV_CLIENT_INPUT,
            /*
                UINT16 COMPAT
            */

            SV_UPDATE_PHYSICS_OBJECT,
            /*
                UINT16 AMOUNT
                0: {
                    UINT16 ID
                    UINT8 FRAMES
                    0: {
                        FLOAT X
                        FLOAT Y
                    }
                    ...
                    n {
                        FLOAT X
                        FLOAT Y
                    }
                }
                ...
                n: {
                    UINT16 ID
                    UINT8 FRAMES
                    0: {
                        FLOAT X
                        FLOAT Y
                    }
                    ...
                    n {
                        FLOAT X
                        FLOAT Y
                    }
                }
            */

            SV_UPDATE_WORLD_SIMULATION_PROPS, // ACK
            /*
                FLOAT TIMESCALE
                UINT16 TICKRATE
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
                    UINT8 SLOT
                    UINT8 TYPE[0]
                    UINT16 ID[0]
                }
                ...
                n: {
                    UINT8 SLOT
                    UINT8 TYPE[n]
                    UINT16 ID[n]
                }

            */
            SV_ENTITY_USE_SKILL_ITEM, // ACK
            /*
                UINT16 ENTID(USER)
                UINT8 TYPE
                UINT32 OBJECTID (skill or slot)
                UINT16 ENTID(TARGET)    // '0' is none
                FLOAT XP                // usually same x as user
                FLOAT YP                // usually same y as user
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
            SV_ADD_ITEM, // ACK
            /*
                UINT16 ENTID
                UINT16 ITEMID
                UINT16 SLOTID
                UINT16 AMNT
                UINT8 UPGRADELV
                UINT16[GAME_ITEM_MAX_COMPOUND_SLOTS] COMPOUND
            */
            SV_REMOVE_ITEM, // ACK
            /*
                UINT16 ENTID
                UINT16 ITEMID
                UINT16 SLOTID
                UINT16 AMNT                
            */
            SV_UPDATE_ENTITY_STANCE_STATE,
            /*
                UINT16 ENTID
                UINT8 FACEDIRECTION
                0: {
                    UINT8 STATE
                    UINT8 NUMBER
                    UINT64 EXPECTEDTIME
                }
                ...
                1: {
                    UINT8 STATE
                    UINT8 NUMBER
                    UINT64 EXPECTEDTIME
                }

            */
            SV_UPDATE_ENTITY_SET_CASTING_STATE, // ACK
            /*
                UINT16 ENTID
                UINT32 ID
                UINT8 TYPE
                UINT16 TARGET
                UINT64 STARTTIME
                UINT64 TIME
                FLOAT X
                FLOAT Y
            */
            SV_UPDATE_ENTITY_ALL_STAT, // ACK
            /*
                UINT16 ENTID
                {
                    sizeof(HealthStat)
                    sizeof(BaseStat)
                    sizeof(ComplexCStat)
                }
            */
            SV_UPDATE_ENTITY_BASE_STAT, // ACK
            /*
                UINT16 ENTID
                {
                    sizeof(BaseStat)
                }
            */
            SV_UPDATE_ENTITY_HEALTH_STAT, // ACK
            /*
                UINT16 ENTID
                {
                    sizeof(HealthStat)
                }
            */
            SV_UPDATE_ENTITY_COMPLEX_STAT, // ACK
            /*
                UINT16 ENTID
                {
                    sizeof(ComplexStat)
                }
           */
           SV_UPDATE_ENTITY_INVENTORY_SLOTS, // ACK
           /*
                UINT16 ENTID
                {
                    sizeof(UINT16) * EquipSlot::TOTAL (16 bytes)
                }
           */
            SV_UPDATE_ENTITY_INVENTORY_CARRY, // ACK
            /*
                UINT16 ENTID      
                UINT8 N     
	            {
                    UINT16 SLOTID
                    UINT16 ITEMID
                    UINT16 QTY
                    UINT8 UPGLV
                    UINT16[GAME_ITEM_MAX_COMPOUND_SLOTS] COMPOUND
                }
                ...
                n: {
                    UINT16 SLOTID
                    UINT16 ITEMID
                    UINT16 QTY
                    UINT8 UPGLV
                    UINT[GAME_ITEM_MAX_COMPOUND_SLOTS] COMPOUND
                }
                {
                    sizeof(UINT16) * EquipSlot::TOTAL (16 bytes)
                }                   
            */
            SV_UPDATE_SKILL_STATE, // ACK
            /*
                UINT16 ENTID
                UInt16 SKILLID
                {
                    CUSTOM PAYLOAD // Ideally less than 32 bytes
                }
            */
            SV_NOTIFY_ENTITY_DEATH, // ACK
            /*
                UINT16 ENTID
                {
                    sizeof(HealthStat)
                }
            */
            SV_NOTIFY_ENTITY_REVIVE, // ACK
            /*
                UINT16 ENTID
                {
                    sizeof(HealthStat)
                }
            */
            SV_SET_GAME_OVER,  // ACK
            SV_SET_GAME_RESTART,  // ACK

            SV_UPDATE_TRAP_STATE,
            /*
                UINT16 ID
                UINT16 STATE
            */   

            SV_UPDATE_MANY_TRAPS_STATE,
            /*
                UINT16 N
                0: {
                    UINT16 ID
                    UINT8 STATE
                }
                ...
                n: { 
                    UINT16 ID
                    UINT8 STATE
                }
            */                            

            SV_NOTIFY_ENTITY_DAMAGE,
            /*
                UINT16 ID
                UINT32 AMNT
                FLOAT X
                FLOAT Y
            */ 

            SV_SET_OBJECT_POSITION,
            /*
                UINT16 ID
                FLOAT X
                FLOAT Y
            */            

        };


    }


#endif
