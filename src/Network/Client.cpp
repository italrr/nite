#include "../Engine/Tools/Tools.hpp"
#include "Client.hpp"

Game::Client::Client() : Game::Net(){
    init = false;
    clear();
}

Game::Client::~Client(){

}

void Game::Client::clear(){
    svOrder = 0;
    serverId = 0;
    lastOrder = 0;
    connected = false;
    sock.close();
    deliveries.clear();
    setState(Game::NetState::Disconnected); 
}

void Game::Client::connect(const String &ip, UInt16 port){
    if(connected){
        nite::print("[client] disconnect from current server");
        return;
    }
    auto server = nite::IP_Port(ip, port);
    this->sv = server;
    if(!init){
        nite::print("[client] failed to connect to "+server.str()+": client must be setup");
        return;
    }    
    if(!sock.open(nite::NetworkDefaultPort + nite::randomInt(1, 200))){
        nite::print("[client] failed to open UDP Socket. cannot connect to "+this->sv.str());
        return;
    }
    lastPacketTimeout = nite::getTicks();
    sock.setNonBlocking(true);
    nite::Packet connect;
    connect.setHeader(Game::PacketType::SV_CONNECT_REQUEST);
    connect.write((char*)&Game::CLIENT_VERSION, sizeof(UInt32));
    connect.write(this->nickname);
    sock.send(server, connect);
    setState(Game::NetState::Connecting);
    nite::print("[client] connecting to "+ip+":"+nite::toStr(port));
    
}

void Game::Client::connect(){
    connect("127.0.0.1", nite::NetworkDefaultPort);
}

void Game::Client::setup(const String &nickname){
    this->nickname = nickname;
    this->init = true;
}

void Game::Client::disconnect(){
    disconnect("");
}

void Game::Client::disconnect(const String &reason){
    if(!init || state == Game::NetState::Disconnected){
        return;
    }
    nite::Packet disc(++lastOrder);
    disc.setHeader(Game::PacketType::SV_CLIENT_DISCONNECT);
    disc.write(reason);
    sock.send(sv, disc);
    nite::print("[client] disconnected from "+sv.str());
    clear();
}

void Game::Client::step(){
    // exit if no connection
    if(state == Game::NetState::Disconnected || !init){
        return;
    }
    nite::Packet handler;
    nite::IP_Port sender;
    if(sock.recv(sender, handler)){
        UInt64 netId = sender.address+sender.port;
        bool isSv = netId == this->serverId && state != Game::NetState::Disconnected;
        if(isSv){
            lastPacket = handler;
            lastPacketTimeout = nite::getTicks();
        }
        switch(handler.getHeader()){
            /*
                SV_ACK
            */             
            case Game::PacketType::SV_ACK: {
                ack(handler);
            } break;
            /*
                SV_PONG
            */      
            case Game::PacketType::SV_PONG: {
                if(!isSv){
                    break;
                }
                this->ping = nite::getTicks() - this->lastPing;           
            } break;             
            /*
                SV_PING
            */             
            case Game::PacketType::SV_PING: {
                if(!isSv){
                    break;
                }
                nite::Packet pong(++lastOrder);
                pong.setHeader(Game::PacketType::SV_PONG);
                sock.send(this->sv, pong);                
            } break;
            /*
                SV_CLIENT_DROP
            */             
            case Game::PacketType::SV_CLIENT_DROP: {
                if(!isSv){
                    break;
                }
                String reason;
                handler.read(reason);
                if(reason.length() == 0){
                    reason = "no reason";
                }
                nite::print("[client] server dropped connection: "+reason);
                clear();               
            } break;                        
            /*
                SV_CONNECT_REJECT
            */             
            case Game::PacketType::SV_CONNECT_REJECT: {
                if(state != Game::NetState::Connecting){
                    break;
                }
                String reason;
                handler.read(reason);
                nite::print("[client] connection rejected: "+reason);
                clear();             
            } break;
            /*
                SV_CONNECT_ACCEPT
            */             
            case Game::PacketType::SV_CONNECT_ACCEPT: {
                if(state == Game::NetState::Connecting){
                    this->sv = sender;
                    this->serverId = netId;
                    this->connected = true;
                    handler.read(&clientId, sizeof(UInt64));
                    setState(Game::NetState::Connected);
                    nite::print("[client] connected to "+this->sv.str()+" | clientId "+nite::toStr(this->clientId));                    
                }
                sendAck(sender, handler.getOrder());
            } break;
        }
    }
    // timeout
    if(nite::getTicks()-lastPacketTimeout > ClientTimeout && this->connected){
        nite::print("[client] disconnected from "+sv.str()+": timeout");
        clear();
        return;
    }
    // ping
    if(nite::getTicks()-lastPing > 1000){
        lastPing = nite::getTicks();
        nite::Packet ping(++lastOrder);
        ping.setHeader(Game::PacketType::SV_PING);
        sock.send(this->sv, ping);
    }
    // timeout connecting
    if(!this->connected && state == Game::NetState::Connecting && nite::getTicks()-lastState > Game::ConnectingTimeout){
        nite::print("[client] failed to connect to "+sv.str()+": timeout");
        clear();
        return;
    }
    updateDeliveries();
}