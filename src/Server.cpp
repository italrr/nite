#include "Server.hpp"
#include "Network.hpp"

Server::Server(){
    running = false;
    genId = nite::randomInt(10, 20);
    setState(ST_WAITING_FOR_PLAYERS);
}

bool Server::listen(const nite::IP_Port &ip){
    if(running){
        nite::print("server is already running. please close it down first.");
        return false;        
    }
    if(!sock.openConn(ip)){
        return false;
    }
    sock.setNonBlocking(true);
    running = true;
    setState(ST_WAITING_FOR_PLAYERS);
    return true;
}

void Server::setState(UInt8 nstate){
    setState(nstate, 0, 0);
}

void Server::setState(UInt8 nstate, UInt8 who, Int32 stateArg){
    this->lastState = nite::getTicks();
    this->state = nstate;    
    nite::Packet statech;
    statech.setHeader(SV_GAME_STATE);
    statech.write(&this->state, sizeof(this->state));
    statech.write(&who, sizeof(who));
    statech.write(&stateArg, sizeof(stateArg));
    sendAll(statech);
    for(int i = 0; i < onChangeCallback.size(); ++i){
        onChangeCallback[i](nstate);
    }    
}

void Server::sendAll(nite::Packet &packet){
    for(auto &it : clients){
        auto &client = it.second;
        this->sock.sendData(client->socket, packet);
    }
}

void Server::init(const Shared<GameState> &game){
    this->game = game;
    this->game->net = this;
}

void Server::clear(){
    sock.close();
    running = false;
    genId = nite::randomInt(10, 20);
    setState(ST_WAITING_FOR_PLAYERS);
}

void Server::update(){
    if(!running){
        return;
    }
    // accept new client
    nite::IP_Port pIP;
    auto nClSocket = sock.acceptConn(pIP);
    if(nClSocket != -1){
        auto player = std::make_shared<ClientProxy>(ClientProxy());
        player->socket = nClSocket;
        player->id = ++genId;
        player->order = this->clients.size();
        this->clients[player->id] = player;
    }
    // receive
    nite::Packet buffer;
    for(auto &it : clients){
        auto &client = it.second;
        buffer.clear();
        auto rcv = sock.recvData(client->socket, buffer);
        if(rcv > 0){
            switch(buffer.getHeader()){
                case SV_JOIN: {
                    buffer.read(client->nickname);
                    nite::print("Player '"+client->nickname+"' just joined");
                    // TODO: handle nicks
                    game->setupPlayer(client);
                    nite::Packet accept;
                    accept.setHeader(SV_ACCEPT);
                    accept.write(client->nickname);
                    accept.write(&client->id, sizeof(client->id));
                    accept.write(game->gType->name);
                    UInt8 nplayers = this->clients.size();;
                    accept.write(&nplayers, sizeof(nplayers));
                    for(auto &it : this->clients){
                        accept.write(it.second->nickname);
                        accept.write(&it.second->id, sizeof(it.second->id));
                        accept.write(it.second->money);
                    }
                    sock.sendData(client->socket, accept);
                } break;
                case SV_PING: {
                    nite::Packet pong;
                    pong.setHeader(SV_PONG);
                    sock.sendData(client->socket, pong);                    
                };
                case SV_PONG: {
                    client->lastPing = nite::getTicks();
                }; 
            }
        }
        if(nite::getTicks()-client->lastPing > 5000){
            sock.drop(client->socket);
            nite::print("client '"+client->nickname+"': lost connection");
            this->clients.erase(it.first);
            continue;
        }        
        if(nite::getTicks()-client->lastSentPing > 500){
            nite::Packet ping;
            ping.setHeader(SV_PING);
            sock.sendData(client->socket, ping);   
            client->lastSentPing = nite::getTicks();         
        }
    }
    // update
    game->update();
}