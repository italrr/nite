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
    sentOrder = 1;
    rcvOrder = 0;
    serverId = 0;
    connected = false;
    sock.close();
    clients.clear();
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
    connect.setOrder(0);
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
    nite::Packet disc(++sentOrder);
    disc.setHeader(Game::PacketType::SV_CLIENT_DISCONNECT);
    disc.write(reason);
    sock.send(sv, disc);
    nite::print("[client] disconnected from "+sv.str());
    clear();
}

void Game::Client::sendChatMsg(const String &msg){
    if(!init || state != Game::NetState::Connected){
        return;
    }
    nite::Packet pack(++svOrder);
    pack.setHeader(Game::PacketType::SV_CHAT_MESSAGE);
    pack.write(&clientId, sizeof(UInt64));
    pack.write(msg);
    persSend(sv, pack, 1500, 4);
}

void Game::Client::update(){
    // exit if no connection
    if(state == Game::NetState::Disconnected || !init){
        return;
    }
    nite::Packet handler;
    nite::IP_Port sender;
    if(sock.recv(sender, handler)){
        UInt64 netId = sender.address + sender.port + sock.getSock();
        bool isSv = netId == this->serverId && state != Game::NetState::Disconnected;
        bool isLast = isLast && handler.getOrder() > rcvOrder;
        if(isSv && isLast){
            lastPacket = handler;
            lastPacketTimeout = nite::getTicks();
            rcvOrder = handler.getOrder();
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
                nite::Packet pong(++sentOrder);
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
                    handler.read(serverName);
                    Game::ClClient yourself;
                    yourself.uid = clientId;
                    yourself.nickname = nickname;
                    this->clients[clientId] = yourself;
                    setState(Game::NetState::Connected);
                    nite::print("[client] connected to "+this->sv.str()+" | clientId "+nite::toStr(this->clientId));                    
                }
                sendAck(sender, handler.getOrder(), ++sentOrder);
            } break;
            /*
                SV_CLIENT_JOIN
            */
            case Game::PacketType::SV_CLIENT_JOIN: {
                UInt64 uid;
                String nick;
                handler.read(&uid, sizeof(UInt64));
                handler.read(nick);
                this->clients[uid] = Game::ClClient(uid, nick);
                nite::print(nick+" joined the game");
                // TODO: add in-game notification for this message
                sendAck(sender, handler.getOrder(), ++sentOrder);
            } break;
            /*
                SV_NOTI_CLIENT_DROP
            */
            case Game::PacketType::SV_NOTI_CLIENT_DROP: {
                UInt64 uid;
                String reason;
                handler.read(&uid, sizeof(UInt64));
                handler.read(reason);
                auto it = clients.find(uid);
                if(it != clients.end()){
                    nite::print(it->second.nickname+" left the game: "+reason);
                    clients.erase(it);
                }
                // TODO: add in-game notification for this message
                sendAck(sender, handler.getOrder(), ++sentOrder);
            } break;            
            /*
                SV_CLIENT_LIST
            */
           case Game::PacketType::SV_CLIENT_LIST: {
               clients.clear();
               UInt16 n;
               handler.read(&n, sizeof(UInt16));
               for(int i = 0; i < n; ++i){
                    Game::ClClient player;           
                    handler.read(&player.uid, sizeof(UInt64));
                    handler.read(&player.ping, sizeof(UInt64));
                    handler.read(player.nickname);  
                    clients[player.uid] = player;
               }
               sendAck(this->sv, handler.getOrder(), ++sentOrder);
           } break;
            /*
                SV_BROADCAST_MESSAGE
            */
           case Game::PacketType::SV_BROADCAST_MESSAGE: {
               String msg;
               handler.read(msg);
               nite::print(msg);
               // TODO: add in-game notification for this message (and chat)
               sendAck(this->sv, handler.getOrder(), ++sentOrder);
           } break;   
            /*
                SV_CHAT_MESSAGE
            */
           case Game::PacketType::SV_CHAT_MESSAGE: {
                UInt64 uid;
                String msg;
                handler.read(&uid, sizeof(UInt64));
                handler.read(msg);
                auto it = clients.find(uid);
                String who = it != clients.end() ? it->second.nickname : "???";
                nite::print(this->nickname+" ["+who+"] "+msg);
                // TODO: add  in-game chat
                sendAck(this->sv, handler.getOrder(), ++sentOrder);
           } break;                    
        }
    }
    // timeout
    if(nite::getTicks()-lastPacketTimeout > ClientTimeout && this->connected){
        nite::print("[client] disconnected from "+sv.str()+": timeout");
        clear();
        return;
    }
    // // ping
    if(nite::getTicks()-lastPing > 250){
        lastPing = nite::getTicks();
        nite::Packet ping(++sentOrder);
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