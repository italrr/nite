#include "Client.hpp"

Client::Client(){
    connected = false;
    id = 0;
    lastSentPing = nite::getTicks();
    lastPing = nite::getTicks();
    setState(ST_WAITING_FOR_PLAYERS);
}

bool Client::connect(const nite::IP_Port &ip){
    if(connected){
        nite::print("client is currently connected to a server. please disconnect first.");
        return false;
    }
    if(!sock.openSocket()){
        return false;
    }    
    if(!sock.connectTo(ip)){
        return false;
    } 
    sock.setNonBlocking(true);
    nite::Packet join;
    join.setHeader(SV_JOIN);
    join.write(nickname);
    sock.sendData(sock.sock, join);
    connected = true;
    setState(ST_WAITING_FOR_PLAYERS);
    return true;
}

void Client::init(const Shared<GameState> &game, const String &nickname){
    this->nickname = nickname;
    this->game = game;
    this->game->net = this;
}

void Client::clear(){
    setState(ST_WAITING_FOR_PLAYERS);
    connected = false;
    sock.close();
    id = 0;
}

void Client::update(){
    game->game();
    if(!connected){
        return;
    }
    nite::Packet buffer;
    auto rcv = sock.recvData(sock.sock, buffer);
    if(rcv > 0){
        switch(buffer.getHeader()){
            case SV_ACCEPT: {
                String gameType;
                UInt8 players;
                buffer.read(this->nickname);
                buffer.read(&this->id, sizeof(this->id));
                buffer.read(gameType);
                buffer.read(&players, sizeof(players));
                for(int i = 0; i < players; ++i){
                    auto client = std::make_shared<ClientProxy>(ClientProxy());
                    buffer.read(client->nickname);
                    buffer.read(&client->id, sizeof(client->id));
                    buffer.read(&client->money, sizeof(client->money));
                    this->clients[client->id] = client;
                }
                nite::print("joined game. your nick is '"+nickname+"'");
            } break;
            case SV_GAME_STATE: {
                UInt8 currentState, who;
                Int32 statearg;
                buffer.read(&currentState, sizeof(currentState));
                buffer.read(&who, sizeof(who));
                buffer.read(&statearg, sizeof(statearg));
                setState(currentState);
                this->lastStateArg = statearg;
                this->lastStateWho = who;
                nite::print("server changed state: "+nite::toStr(currentState));
            } break;
            case SV_REJECT: {
                String reason;
                buffer.read(reason);
                nite::print("connection rejected: "+reason);
                clear();
                return;
            } break;
            case SV_PING: {
                nite::Packet pong;
                pong.setHeader(SV_PONG);
                sock.sendData(sock.sock, pong);                    
            };            
            case SV_PONG: {
                this->lastPing = nite::getTicks();
            } break;
        }        
    }
    if(nite::getTicks()-lastPing > 5000){
        nite::print("server connection lost");
        clear();
        return;
    }
    if(nite::getTicks()-lastSentPing > 500){
        nite::Packet ping;
        ping.setHeader(SV_PING);
        sock.sendData(sock.sock, ping);
        lastSentPing = nite::getTicks();
    }
    game->update();
}

void Client::draw(){
    // draw
    nite::setRenderTarget(nite::RenderTargetGame);
    game->board->draw(0, 0);
    nite::Vec2 size(1080, 1080);
    game->dice->draw(size.x * 0.80f, size.y * 0.70f, true);
}