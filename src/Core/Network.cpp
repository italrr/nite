#include "Network.hpp"
#include "../Engine/Tools/Tools.hpp"
#include "../Engine/Tools/hashids.hpp"

Game::Net::Net(){
    lastInitTfId = 0;
    setState(Game::NetState::Disconnected);
    clock.set(nite::getTicks());
    stateDeltas.reserve(1000);
    deltaUpdate = nite::getTicks();
}

void Game::Net::setState(unsigned state){
    this->lastState = nite::getTicks();
    this->state = state;
}

Shared<Game::PersisentDelivey> Game::Net::persSend(nite::IP_Port &client, nite::Packet &packet){
    return persSend(client, packet, 250, 3);
}

Shared<Game::PersisentDelivey> Game::Net::persSend(nite::IP_Port &client, nite::Packet &packet, UInt64 retryInterval, int retries){
    if(!init){
        nite::print("persend when net is not init");
    }    
    UInt64 netId = client.address + client.port + sock.sock;
    dropPersForHeader(netId, packet.getHeader()); // remove older versions of this packet
    auto pd = Shared<Game::PersisentDelivey>(new Game::PersisentDelivey());
    pd->retryInterval = retryInterval;
    pd->retries = retries;
    pd->retry = 0; 
    pd->netId = netId;
    pd->packet = packet;
    pd->cl = client;
    pd->order = packet.getOrder();
    pd->lastRetry = nite::getTicks();
    pd->created = nite::getTicks();
    sock.send(client, packet);
    deliveries.push_back(pd);
    return pd;
}

void Game::Net::updateDeliveries(){
    clock.update();
    if(!init){
        return;
    }    
    // send deliveries
    for(int i = 0; i < deliveries.size(); ++i){
        auto del = deliveries[i].get();
        if(!del->stale && nite::getTicks()-del->lastRetry > del->retryInterval){
            ++del->retry;
            del->lastRetry = nite::getTicks();
            sock.send(del->cl, del->packet);
        }
    }
    // mark expired ones as stale
    for(int i = 0; i < deliveries.size(); ++i){
        auto del = deliveries[i].get();
        // only drops packages with positive retries
        // also, if a client doesn't respond a packet after 30 seconds of retries the packet is dropped
        if(!del->stale && del->retries != -1 && del->retry >= del->retries || (nite::getTicks()-del->created  > 60 * 1000)){ 
            del->markStale();
        }
    }   
    // drop stale ones
    for(int i = 0; i < deliveries.size(); ++i){
        auto del = deliveries[i].get();
        if(del->stale && nite::getTicks()-del->lastStaleTick > 10 * 1000){ 
            deliveries.erase(deliveries.begin() + i);
            --i;
        }
    }        
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

void Game::Net::dropPersFor(UInt64 netId){
    for(int i = 0; i < deliveries.size(); ++i){
        auto del = deliveries[i].get();
        if(!del->stale && del->netId == netId){
            del->markStale();
        }
    }
}

void Game::Net::dropPersForHeader(UInt64 netId, UInt16 header){
    for(int i = 0; i < deliveries.size(); ++i){
        auto del = deliveries[i].get();
        if(!del->stale && del->netId == netId && del->packet.getHeader() == header){
            del->markStale();
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
        auto pck = deliveries[i].get();
        if(!pck->stale && pck->order == order){
            pck->onAck(pck->onAckPayload, pck->cl);
            pck->markStale();
        }
    }
}

void Game::Net::bindOnAckFor(UInt16 header, std::function<void(nite::SmallPacket &payload, nite::IP_Port &cl)> lambda, nite::SmallPacket packet){
    for(int i = 0; i < deliveries.size(); ++i){
        auto pck = deliveries[i].get();
        if(!pck->stale && pck->packet.getHeader() == header){
            pck->onAckPayload = packet;
            pck->onAck = lambda;
        }
    }
}

// this one is to avoid copying nite::SmallPacket if it is not needed
void Game::Net::bindOnAckFor(UInt16 header, std::function<void(nite::SmallPacket &payload, nite::IP_Port &cl)> lambda){
    for(int i = 0; i < deliveries.size(); ++i){
        auto pck = deliveries[i].get();
        if(!pck->stale && pck->packet.getHeader() == header){         
            pck->onAck = lambda;
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

void Game::Net::issueStateDeltaUpdate(const Game::StateDelta &cmd){
    this->stateDeltas.push_back(cmd);
}

void Game::Net::setCurrentMap(Shared<nite::Map> &m){
    traps.clear();
    world.clear();
    map = Shared<nite::Map>(NULL);
    nite::Vec2 ws = m->size * m->tileSize;
    this->world.setSize(ws.x, ws.y, 64, 64);
    // wall masks
    for(int i = 0; i < m->masks.size(); ++i){
        auto &mask = m->masks[i];
        auto obj = new Game::NetObject();
        obj->position = mask.position;
        obj->objType = ObjectType::Wall;
        obj->sigId = ObjectType::Wall;
        obj->size = mask.size;
        obj->solid = true;
        obj->unmovable = true;
        this->world.addWallMask(obj); // we use id 0 for wallmasks as these are not managed by the server
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
            nite::print("failed create trap '"+sp.tag+"': it's undefined");
            continue;
        }
        t->position = sp.position;
        t->size = sp.size;
        t->dynTile = sp.ref;
        traps.add(t, sp.id);
    }    
    nite::print("[net] dynamic tiles: "+nite::toStr(m->dynamicTiles.size())+" | cmasks: "+nite::toStr(this->world.wallMasks.size())+" | gmasks: "+nite::toStr(this->world.ghostMasks.size()));
}
