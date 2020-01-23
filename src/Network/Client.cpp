#include "../Engine/Tools/Tools.hpp"
#include "Client.hpp"

Game::Client::Client() : Game::Net(){
    init = false;
}

Game::Client::~Client(){

}

void Game::Client::connect(const String &ip, UInt16 port){
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
    sock.setNonBlocking(true);
    nite::Packet connect;
    connect.setHeader(Game::PacketType::SV_CONNECT_REQUEST);
    connect.write((char*)&Game::CLIENT_VERSION, sizeof(UInt32));
    connect.write(this->clientId);
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
    nite::print("[client] client ID: "+this->generateId());
}

void Game::Client::step(){
    // exit if no connection
    if(state == Game::NetState::Disconnected || !init){
        return;
    }
    nite::Packet handler;
    nite::IP_Port sender;
    if(sock.recv(sender, handler)){
        switch(handler.getHeader()){
            case Game::PacketType::SV_CONNECT_REJECT: {
                if(state == Game::NetState::Connecting){
                    String reason;
                    handler.read(reason);
                    nite::print("[client] connection rejected: "+reason);
                    setState(Game::NetState::Disconnected);
                }
            } break;
            case Game::PacketType::SV_CONNECT_ACCEPT: {
                this->sv = sender;
                handler.read(serverId);
                setState(Game::NetState::Connected);
                nite::print("[client] connected to"+this->sv.str());
            } break;
        }
    }
    // time out connecting
    if(state == Game::NetState::Connecting && nite::getTicks()-lastState > Game::ConnectingTimeout){
        nite::print("[client] failed to connect to "+sv.str()+": timeout");
        setState(Game::NetState::Disconnected);
        return;
    }
}