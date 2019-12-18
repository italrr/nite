#ifndef ENTITY_HPP
  #define ENTITY_HPP
    #include "../Types.hpp"
    #include "../Network.hpp"
    #include "../Packets.hpp"
    #include "World.hpp"

    struct clientT {
      clientT();
      nite::IP_Port IP;
      int socket;
      String name;
      unsigned joinTime;
      unsigned dropTime;
      unsigned lastPing;
      bool empty;
      bool Confirmed;
      bool Dropped;
    };

    struct packetQueueT {
      clientT Client;
      nite::Packet Packet;
      packetQueueT(const clientT &Client, const nite::Packet &Packet);
    };

    struct serverT {
      Vector<packetQueueT> packetQueue;
      nite::TCPSocket TCP;
      unsigned Port;
      bool Running;
      String name;
      int getEmptySlot();
      Vector<clientT> Clients;
      worldT World;
      void dropClient(clientT &Client, const String &Reason);
      void parsePackets(clientT &Client);
      clientT connectRequest(int Sock, const nite::IP_Port &NC);
      void init(const String &name, unsigned Port);
      void end();
      void update();
      serverT();
      ~serverT();
    };

#endif
