#include "Server.hpp"
#include "../Engine/Tools/Tools.hpp"

Game::Server::Server(){
    this->init = false;
}

Game::Server::~Server(){

}

void Game::Server::listen(UInt8 maxClients, UInt16 port){
    if(maxClients == 0){
        maxClients = 4;
    }
    this->maxClients = maxClients;
    if(!sock.open(nite::NetworkDefaultPort)){
        nite::print("[server] failed to start server");
        return;
    }
    sock.setNonBlocking(true);
    nite::print("[server] started server | max clients "+nite::toStr(this->maxClients)+" | listening at "+nite::toStr(port));
    nite::print("[server] waiting for clients...");
    this->init = true;
}

Game::SvClient *Game::Server::getClient(UInt64 uid){
    if(clients.count(uid) > 0){
        return &clients[uid];
    }
    return NULL;
}

Game::SvClient *Game::Server::getClient(const String &nick){
    for(auto _cl : clients){
        if(_cl.second.nickname == nick){
            return &clients[_cl.first];
        }
    }
    return NULL;
}

void Game::Server::dropClient(UInt64 uid, String reason){
    auto cl = this->getClient(uid);
    if(cl == NULL){
        nite::print("[server] cannot drop client "+nite::toStr(uid)+": it's not connected");
        return;
    }
    nite::Packet drop(++cl->lastOrder);
    drop.setHeader(Game::PacketType::SV_CLIENT_DROP);
    drop.write(reason);
    sock.send(cl->cl, drop);
    removeClient(uid);
}

void Game::Server::dropClient(UInt64 uid){
    dropClient(uid, "no reason");
}

void Game::Server::removeClient(UInt64 uid){
    auto client = getClient(uid);
    if(!client){
        return;
    }
    auto it = clients.find(uid);
    clients.erase(it);
}

void Game::Server::step(){
    if(!init){
        return;
    }
    nite::IP_Port sender;
    nite::Packet handler;
    if(sock.recv(sender, handler)){
        UInt64 netId = sender.address+sender.port;
        auto client = this->getClient(netId);
        bool isLast = client && client->lastOrder > handler.getOrder();
        if(client && isLast){
            client->lastPacketTimeout = nite::getTicks();
            client->lastPacket = handler;
            client->lastOrder = handler.getOrder();
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
                if(!client){
                    break;
                }
                client->ping = nite::getTicks() - client->lastPing; 
            } break; 
            /*
                SV_CLIENT_DISCONNECT
            */             
            case Game::PacketType::SV_CLIENT_DISCONNECT: {
                if(!client){
                    break;
                }
                String reason;
                handler.read(reason);
                if(reason.length() == 0){
                    reason = "no reason";
                }
                nite::print("[server] disconnected clientId "+nite::toStr(netId)+": "+reason);
                removeClient(netId);             
            } break;                  
            /*
                SV_PING
            */             
            case Game::PacketType::SV_PING: {
                if(!client){
                    break;
                }
                nite::Packet pong(++client->lastOrder);
                pong.setHeader(Game::PacketType::SV_PONG);
                sock.send(client->cl, pong);                
            } break;                             
            /*
                SV_CONNECT_REQUEST
            */
            case Game::PacketType::SV_CONNECT_REQUEST : {
                nite::print("[server] incoming connection...");
                if(clients.size() == maxClients){
                    nite::Packet reject;
                    reject.setHeader(Game::PacketType::SV_CONNECT_REJECT);
                    reject.write("server is full");
                    nite::print("[server] rejected client "+nite::toStr(netId)+" : server full");
                    sock.send(sender, reject); 
                    break;
                }
                UInt32 clVer;
                String nick;
                handler.read(&clVer, sizeof(UInt32));
                handler.read(nick);
                // TODO: handle repeated nicks
                if(client){
                    dropClient(netId, "client of same of origin");
                }else{
                    Game::SvClient cl;
                    cl.nickname = nick;
                    cl.clientId = netId;
                    cl.lastPacketTimeout = nite::getTicks();
                    cl.role = Game::SvClientRole::Player;
                    cl.cl = sender;
                    clients[netId] = cl;
                    nite::Packet accepted(++cl.svOrder);
                    accepted.setHeader(Game::PacketType::SV_CONNECT_ACCEPT);
                    accepted.write(&netId, sizeof(UInt64));
                    persSend(cl.cl, accepted);
                    nite::print("[server] accepted clientId "+nite::toStr(netId)+" | nick '"+nick+"'");
                }
                
            } break;           
        }
    }

    // timeout
    Vector<UInt64> timedoutClients;
    for(auto cl : clients){
        if(nite::getTicks()-cl.second.lastPacketTimeout > ClientTimeout){
            timedoutClients.push_back(cl.first);
        }
    }
    for(int i = 0; i < timedoutClients.size(); ++i){
        removeClient(timedoutClients[i]);
        nite::print("[server] dropped clientId "+nite::toStr(timedoutClients[i])+": timeout");
    }

    // ping
    for(auto cl : clients){
        auto &client = clients[cl.first];
        if(nite::getTicks()-client.lastPing > 1000){
            client.lastPing = nite::getTicks();
            nite::Packet ping(++client.lastOrder);
            ping.setHeader(Game::PacketType::SV_PING);
            sock.send(client.cl, ping);
        }
    }

    updateDeliveries();
}