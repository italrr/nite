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

Game::PersisentDelivey& Game::Net::persSend(nite::IP_Port &client, nite::Packet &packet){
    return persSend(client, packet, 250, 3);
}

Game::PersisentDelivey& Game::Net::persSend(nite::IP_Port &client, nite::Packet &packet, UInt64 retryInterval, int retries){
    if(!init){
        nite::print("persend when net is not init");
    }    
    UInt64 netId = client.address + client.port + sock.getSock();
    dropPersForHeader(netId, packet.getHeader()); // remove older versions of this packet
    Game::PersisentDelivey pd;
    pd.retryInterval = retryInterval;
    pd.retries = retries;
    pd.retry = 0; 
    pd.netId = netId;
    pd.packet = packet;
    pd.cl = client;
    pd.order = packet.getOrder();
    pd.lastRetry = nite::getTicks();
    pd.created = nite::getTicks();
    sock.send(client, packet);
    deliveries.push_back(pd);
    return deliveries[deliveries.size() - 1];
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
        // also, if a client doesn't respond a packet after 30 seconds of retries the packet is dropped
        if(del.retries != -1 && del.retry >= del.retries || (nite::getTicks()-del.created  > 30 * 1000)){ 
            deliveries.erase(deliveries.begin() + i);
            --i;
        }
    }    
}

void Game::Net::dropPersFor(UInt64 netId){
    for(int i = 0; i < deliveries.size(); ++i){
        auto &del = deliveries[i];
        if(del.netId == netId){
            deliveries.erase(deliveries.begin() + i);
            --i;
        }
    }
}

void Game::Net::dropPersForHeader(UInt64 netId, UInt16 header){
    for(int i = 0; i < deliveries.size(); ++i){
        auto &del = deliveries[i];
        if(del.netId == netId && del.packet.getHeader() == header){
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
            deliveries[i].onAck(deliveries[i].onAckPayload, deliveries[i].cl);
            deliveries.erase(deliveries.begin() + i);
            --i;
        }
    }
}

void Game::Net::bindOnAckFor(UInt16 header, std::function<void(nite::SmallPacket &payload, nite::IP_Port &cl)> lambda, nite::SmallPacket packet){
    for(int i = 0; i < deliveries.size(); ++i){
        auto &pck = deliveries[i];
        if(pck.packet.getHeader() == header){
            pck.onAckPayload = packet;
            pck.onAck = lambda;
        }
    }
}

// this one is to avoid copying nite::SmallPacket if it is not needed
void Game::Net::bindOnAckFor(UInt16 header, std::function<void(nite::SmallPacket &payload, nite::IP_Port &cl)> lambda){
    for(int i = 0; i < deliveries.size(); ++i){
        auto &pck = deliveries[i];
        if(pck.packet.getHeader() == header){         
            pck.onAck = lambda;
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

