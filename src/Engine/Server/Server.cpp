#include "../Game.hpp"
#include "Server.hpp"
#include "../Tools/Tools.hpp"
#include "Packets.hpp"

using namespace nite;

packetQueueT::packetQueueT(const clientT &Client, const nite::Packet &Packet){
  this->Client = Client;
  this->Packet = Packet;
}

clientT::clientT(){
  empty       = true;
  Confirmed   = false;
}

void serverT::init(const String &name, unsigned Port){
  this->name = name;
  this->Port = Port;
  World.init(400, 400);
  Running = true;
  print("Server '"+name+"' started.");
  TCP.open(Port);
  TCP.setNonBlocking(true);
  TCP.listen(5);
}

void serverT::end(){
  Running = false;
  print("Server '"+name+"' stopped.");
}

clientT serverT::connectRequest(int Sock, const IP_Port &NC){
  clientT Client;
  Client.name      = NC.IP+" "+toStr(NC.Port);
  Client.IP        = NC;
  Client.socketId    = Sock;
  Client.joinTime  = getTocks();
  Client.empty     = false;
  Client.Confirmed = false;
  Client.lastPing  = getTocks();
  return Client;
}

int serverT::getEmptySlot(){
    for(unsigned i=0; i<Clients.size(); ++i){
        if(Clients[i].empty) return i;
    }
    Clients.push_back(clientT());
    return Clients.size()-1;
}

void serverT::dropClient(clientT &Client, const String &Reason){
  Client.Dropped       = true;
  Client.dropTime      = getTocks();
  Packet Dropped;
  Dropped.setHeader(PACKET_CLIENT_DROPPED);
  Dropped.writeString(Reason);
  packetQueue.push_back(packetQueueT(Client, Dropped));
}

void serverT::parsePackets(clientT &Client){
  if (Client.empty) return;
  // Handle if dropped
  if(Client.Dropped){
    if(getTocks()-Client.dropTime>5){
      Client.empty = true;
    }
    return;
  }
  Packet Rev;
  TCP.receive(Client.IP.Port, Rev.Data, Rev.getMax());
  // Confirm
  if(Rev.getHeader() == PACKET_CONNECT_CONFIRM && !Client.Confirm){
    UInt16 revVersion;
    Rev.Read(&revVersion, sizeof(UInt16));
    if(revVersion != Game::Version){
      dropClient(Client, "Server's version mismatches with yours.");
    }else{
      Client.Confirmed = true;
    }
  }
  if(getTocks()-Client.joinTime >=5 && !Client.Confirmed){
    dropClient(Client, "timeout in confirmation.");
  }
  // Parse
  switch(Rev.getHeader()){
    case PACKET_PING:
      Client.lastPing = getTocks();
    break;
  }
  // Drop if ping timeout
  if(getTocks()-Client.lastPing >= 20){
    dropClient(Client, "Connection timeout.");
  }
}

void serverT::update(){
  if(!Running) return;
  // accept new connnections
  IP_Port NC;
  int Sock = TCP.accept(NC);
  if(Sock >= 0){
    Clients[getEmptySlot()] = connectRequest(Sock, NC);
    print("connected client '"+NC.str()+"'.");
  }
  // Parse packets
  for(unsigned i=0; i<Clients.size(); ++i){
    parsePackets(Clients[i]);
  }
  // send Message Queue
  for(unsigned i=0; i<packetQueue.size(); ++i){
    packetQueueT &PK = packetQueue[i];
    TCP.send(PK.Client.socketId, PK.Packet.Data, PK.Packet.getSize());
  }
  packetQueue.clear();
}

serverT::serverT(){

}

serverT::~serverT(){

}
