#include "Network.hpp"
#include "../Engine/Tools/Tools.hpp"
#include "../Engine/Tools/hashids.hpp"

Game::Net::Net(){
    lastInitTfId = 0;
    setState(Game::NetState::Disconnected);
    clock.set(nite::getTicks());
    packetQueue.reserve(1000);
    rcvPackets.reserve(1000);
    gameTickRate = 32;
    currentDelta = 0;
    gameTimescale = 1.0f;
    lastGameUpdate = nite::getTicks();
}

void Game::Net::setState(unsigned state){
    this->lastState = nite::getTicks();
    this->state = state;
}

void Game::RemoteClock::update(){
    time += (nite::getTicks() - lastTick);
    lastTick = nite::getTicks();
}

void Game::RemoteClock::set(UInt64 time){
    this->delta = this->time - time;
    this->time = time;
    this->lastTick = nite::getTicks();
}

void Game::Net::dropPersFor(const nite::IP_Port &ip){
    for(int i = 0; i < deliveries.size(); ++i){
        auto del = deliveries[i].get();
        if(del->ip.isSame(ip)){
            deliveries.erase(deliveries.begin() + i);
            --i;
        }
    }
}

void Game::Net::dropPersForHeader(const nite::IP_Port &ip, UInt16 header){
    for(int i = 0; i < deliveries.size(); ++i){
        auto del = deliveries[i].get();
        if(del->ip.isSame(ip) && del->packet.getHeader() == header){
            deliveries.erase(deliveries.begin() + i);
            --i;
        }
    }
}

void Game::Net::ack(nite::Packet &packet){
    if(!init){
        return;
    }    
    UInt32 ack = packet.getAck();
    for(int i = 0; i < deliveries.size(); ++i){
        auto pck = deliveries[i].get();
        if(pck->ack == ack && pck->ip.isSame(packet.sender)){
            pck->onAck(pck->onAckPayload, packet.sender);
            deliveries.erase(deliveries.begin() + i);
            --i;
        }
    }
}

void Game::Net::updateDeliveries(){
    for(int i = 0; i < deliveries.size(); ++i){
        auto del = deliveries[i].get();
        if(nite::getTicks()-del->lastRetry > 1000){
            del->lastRetry = nite::getTicks();
            sendPacketFor(del->ip, del->packet);
        }
    }    
}

void Game::Net::bindOnAckFor(UInt32 ack, std::function<void(nite::SmallPacket &payload, nite::IP_Port &ip)> lambda, nite::SmallPacket packet){
    for(int i = 0; i < deliveries.size(); ++i){
        auto pck = deliveries[i].get();
        if(pck->ack == ack){
            pck->onAckPayload = packet;
            pck->onAck = lambda;
        }
    }
}

void Game::Net::bindOnAckFor(UInt32 ack, std::function<void(nite::SmallPacket &payload, nite::IP_Port &ip)> lambda){
    for(int i = 0; i < deliveries.size(); ++i){
        auto pck = deliveries[i].get();
        if(pck->ack == ack){     
            pck->onAck = lambda;
        }
    }
}

void Game::Net::bindOnAckForHeader(UInt16 header, std::function<void(nite::SmallPacket &payload, nite::IP_Port &ip)> lambda, nite::SmallPacket packet){
    for(int i = 0; i < deliveries.size(); ++i){
        auto pck = deliveries[i].get();
        if(pck->header == header){
            pck->onAckPayload = packet;
            pck->onAck = lambda;
        }
    }
}

void Game::Net::bindOnAckForHeader(UInt16 header, std::function<void(nite::SmallPacket &payload, nite::IP_Port &ip)> lambda){
    for(int i = 0; i < deliveries.size(); ++i){
        auto pck = deliveries[i].get();
        if(pck->header == header){
            pck->onAck = lambda;
        }
    }
}

void Game::Net::sendAck(nite::IP_Port &ip, UInt32 ackId){
    if(!init){
        return;
    }
    nite::Packet ack(Game::PacketType::SV_ACK);
    ack.setAck(ackId);
    sendPacketFor(ip, ack);
}

void Game::Net::sendPacketFor(const nite::IP_Port &ip, nite::Packet &packet){
    packet.receiver = ip;
    packetQueue.push_back(packet);
}

void Game::Net::sendPacketForMany(const Vector<nite::IP_Port> &ips, nite::Packet &packet){
    for(int i = 0; i < ips.size(); ++i){
        packet.receiver = ips[i];
        packetQueue.push_back(packet);
    }
}

void Game::Net::sendPersPacketFor(const nite::IP_Port &ip, nite::Packet &packet, UInt32 ack){
    packet.receiver = ip;
    packet.setAck(ack);
    packetQueue.push_back(packet);  
    auto pd = Shared<Game::PersisentDelivey>(new Game::PersisentDelivey());
    pd->ip = ip;
    pd->ack = ack;
    pd->header = packet.getHeader();
    pd->packet = packet;
    pd->lastRetry = nite::getTicks();
    deliveries.push_back(pd);     
}

void Game::Net::sendPersPacketForMany(const Vector<nite::IP_Port> &ips, nite::Packet &packet, const Vector<UInt32> acks){
    for(int i = 0; i < ips.size(); ++i){
        auto &ip = ips[i];
        auto &ack = acks[i];
        packet.receiver = ip;
        packet.setAck(ack);
        packetQueue.push_back(packet);        
        auto pd = Shared<Game::PersisentDelivey>(new Game::PersisentDelivey());
        pd->ip = ip;
        pd->header = packet.getHeader();
        pd->ack = ack;
        pd->packet = packet;
        pd->lastRetry = nite::getTicks();
        deliveries.push_back(pd);  
    }
}

void Game::Net::step(){

}

void Game::Net::setCurrentMap(Shared<nite::Map> &m){
    traps.clear();
    world.clear();
    map = Shared<nite::Map>(NULL);
    this->world.init(m->size.x, m->size.y, m->tileSize.x);
    // wall masks
    for(int i = 0; i < m->masks.size(); ++i){
        auto &mask = m->masks[i];
        nite::Vec2 p = mask.position / nite::Vec2(m->tileSize);
        int j = world.toIndex(p);
        if(j < 0 || j > world.total){
            nite::print("[net] invalid collision mask");
            continue;
        }
        world.cells[j] = -1;
    }
    this->map = m;
    // dynamic tiles
    for(auto &it : m->dynamicTiles){
        m->generateDynamicTile(it.second.stateToIndex, it.second.tiles, it.second.layer, it.second.id);
    }    
    // traps
    for(int i = 0; i < m->specials.size(); ++i){
        auto &sp = m->specials[i];
        if(sp.type != "trap"){
            continue;
        }
        auto t = Game::getTrap(sp.tag);
        if(t.get() == NULL){
            nite::print("[net] failed create trap '"+sp.tag+"': it's undefined");
            continue;
        }
        t->position = sp.position;
        t->size = sp.size;
        t->dynTile = sp.ref;
        traps.add(t, sp.id);
    }    
    nite::print("[net] dynamic tiles: "+nite::toStr(m->dynamicTiles.size()));
}
