#include "Network.hpp"
#include "../Engine/Tools/Tools.hpp"
#include "../Engine/Tools/hashids.hpp"

Game::Net::Net(){
    setState(Game::NetState::Disconnected);
}

void Game::Net::setState(unsigned state){
    this->lastState = nite::getTicks();
    this->state = state;
}

void Game::Net::persSend(nite::IP_Port &client, nite::Packet &packet){
    persSend(client, packet, 250, 3);
}

void Game::Net::persSend(nite::IP_Port &client, nite::Packet &packet, UInt64 retryInterval, int retries){
    if(!init){
        return;
    }    
    Game::PersisentDelivey pd;
    pd.retryInterval = retryInterval;
    pd.retries = retries;
    pd.retry = 0; 
    pd.netId = client.address + client.port + sock.getSock();
    pd.packet = packet;
    pd.cl = client;
    pd.order = packet.getOrder();
    pd.lastRetry = nite::getTicks();
    pd.created = nite::getTicks();
    sock.send(client, packet);
    deliveries.push_back(pd);
}

void Game::Net::updateDeliveries(){
    if(!init){
        return;
    }    
    // send deliveries
    for(int i = 0; i < deliveries.size(); ++i){
        auto &del = deliveries[i];
        if(nite::getTicks()-del.lastRetry > del.retryInterval){
            ++del.retry;
            del.lastRetry = nite::getTicks();
            sock.send(del.cl, del.packet);
        }
    }
    // drop expired ones
    for(int i = 0; i < deliveries.size(); ++i){
        auto &del = deliveries[i];
        // only drops packages with positive retries
        if(del.retries != -1 && del.retry >= del.retries){
            deliveries.erase(deliveries.begin() + i);
            --i;
        }
    }    
}

void Game::Net::dropPersFor(UInt64 netId){
    for(int i = 0; i < deliveries.size(); ++i){
        auto &del = deliveries[i];
        if(del.netId = netId){
            deliveries.erase(deliveries.begin() + i);
            --i;
        }
    }
}

void Game::Net::ack(nite::Packet &packet){
    if(!init){
        return;
    }    
    UInt32 order;
    packet.read(&order, nite::NetworkOrderSize);
    for(int i = 0; i < deliveries.size(); ++i){
        if(deliveries[i].order == order){
            deliveries.erase(deliveries.begin() + i);
            --i;
        }
    }
}

void Game::Net::sendAck(nite::IP_Port &client, UInt32 ackId, UInt32 order){
    if(!init){
        return;
    }
    nite::Packet ack;
    ack.setOrder(order);
    ack.setHeader(Game::PacketType::SV_ACK);
    ack.write(&ackId, nite::NetworkOrderSize);
    sock.send(client, ack);
}

void Game::Net::step(){

}

