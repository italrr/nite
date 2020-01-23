#include "Server.hpp"
#include "../Engine/Tools/Tools.hpp"

Game::Server::Server(){
    this->init = false;
    this->serverId = generateId();
}

Game::Server::~Server(){

}

void Game::Server::listen(UInt8 maxClients, UInt16 port){
    if(maxClients == 0){
        maxClients = 0;
    }
    if(!sock.open(nite::NetworkDefaultPort)){
        nite::print("[server] failed to start server");
        return;
    }
    sock.setNonBlocking(true);
    nite::print("[server] started server | max clients "+nite::toStr(maxClients)+" | listening at "+nite::toStr(port));
    nite::print("[server] waiting for clients...");
    init = true;
}

void Game::Server::step(){
    if(!init){
        return;
    }
    nite::IP_Port sender;
    nite::Packet handler;
    if(sock.recv(sender, handler)){
        switch(handler.getHeader()){
            case Game::PacketType::SV_CONNECT_REQUEST : {
                nite::print("[server] incoming connection...");
                if(clients.size() == maxClients){
                    nite::Packet reject;
                    reject.setHeader(Game::PacketType::SV_CONNECT_REJECT);
                    reject.write("server is full");
                    nite::print("[server] rejected client: server full"); // TODO: print clientId
                    sock.send(sender, reject);
                }else{

                }
            } break;
        }
    }
}