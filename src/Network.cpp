#include "Network.hpp"



Vector<nite::IP_Port> NetHandle::getAllClientsIps(){
    Vector<nite::IP_Port> ips;
    for(auto &it : clients){
        ips.push_back(it.second->ip);
    }
    return ips;    
}

Vector<UInt32> NetHandle::getAllClientsAcks(){
    Vector<UInt32> acks;
    for(auto &it : clients){
        acks.push_back(++it.second->svAck);
    }
    return acks;
}

void NetHandle::dropPersFor(const nite::IP_Port &ip){
    for(int i = 0; i < deliveries.size(); ++i){
        auto del = deliveries[i].get();
        if(del->ip.isSame(ip)){
            deliveries.erase(deliveries.begin() + i);
            --i;
        }
    }
}

void NetHandle::dropPersForHeader(const nite::IP_Port &ip, UInt16 header){
    for(int i = 0; i < deliveries.size(); ++i){
        auto del = deliveries[i].get();
        if(del->ip.isSame(ip) && del->packet.getHeader() == header){
            deliveries.erase(deliveries.begin() + i);
            --i;
        }
    }
}


Shared<ClientProxy> NetHandle::getClientByIp(const nite::IP_Port &ip){
    for(auto &it : clients){
        if(it.second->ip == ip){
            return it.second;
        }
    }
    return Shared<ClientProxy>(NULL);
}

void NetHandle::ack(nite::Packet &packet){
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

void NetHandle::updateDeliveries(){
    for(int i = 0; i < deliveries.size(); ++i){
        auto del = deliveries[i].get();
        if(nite::getTicks()-del->lastRetry > 1000){
            del->lastRetry = nite::getTicks();
            sendPacketFor(del->ip, del->packet);
        }
    }    
}

void NetHandle::sendAck(nite::IP_Port &ip, UInt32 ackId){
    nite::Packet ack(SV_ACK);
    ack.setAck(ackId);
    sendPacketFor(ip, ack);
}

void NetHandle::sendPacketFor(const nite::IP_Port &ip, nite::Packet &packet){
    packet.receiver = ip;
    packetQueue.push_back(packet);
}

void NetHandle::sendPacketForMany(const Vector<nite::IP_Port> &ips, nite::Packet &packet){
    for(int i = 0; i < ips.size(); ++i){
        packet.receiver = ips[i];
        packetQueue.push_back(packet);
    }
}

void NetHandle::sendPersPacketFor(const nite::IP_Port &ip, nite::Packet &packet, UInt32 ack){
    packet.receiver = ip;
    packet.setAck(ack);
    packetQueue.push_back(packet);  
    auto pd = Shared<PersisentDelivey>(new PersisentDelivey());
    pd->ip = ip;
    pd->ack = ack;
    pd->header = packet.getHeader();
    pd->packet = packet;
    pd->lastRetry = nite::getTicks();
    deliveries.push_back(pd);     
}

void NetHandle::sendPersPacketForMany(const Vector<nite::IP_Port> &ips, nite::Packet &packet, const Vector<UInt32> acks){
    for(int i = 0; i < ips.size(); ++i){
        auto &ip = ips[i];
        auto &ack = acks[i];
        packet.receiver = ip;
        packet.setAck(ack);
        packetQueue.push_back(packet);        
        auto pd = Shared<PersisentDelivey>(new PersisentDelivey());
        pd->ip = ip;
        pd->header = packet.getHeader();
        pd->ack = ack;
        pd->packet = packet;
        pd->lastRetry = nite::getTicks();
        deliveries.push_back(pd);  
    }
}

void NetHandle::bindOnAckFor(UInt32 ack, std::function<void(nite::SmallPacket &payload, nite::IP_Port &ip)> lambda, nite::SmallPacket packet){
    for(int i = 0; i < deliveries.size(); ++i){
        auto pck = deliveries[i].get();
        if(pck->ack == ack){
            pck->onAckPayload = packet;
            pck->onAck = lambda;
        }
    }
}

void NetHandle::bindOnAckFor(UInt32 ack, std::function<void(nite::SmallPacket &payload, nite::IP_Port &ip)> lambda){
    for(int i = 0; i < deliveries.size(); ++i){
        auto pck = deliveries[i].get();
        if(pck->ack == ack){     
            pck->onAck = lambda;
        }
    }
}

void NetHandle::bindOnAckForHeader(UInt16 header, std::function<void(nite::SmallPacket &payload, nite::IP_Port &ip)> lambda, nite::SmallPacket packet){
    for(int i = 0; i < deliveries.size(); ++i){
        auto pck = deliveries[i].get();
        if(pck->header == header){
            pck->onAckPayload = packet;
            pck->onAck = lambda;
        }
    }
}

void NetHandle::bindOnAckForHeader(UInt16 header, std::function<void(nite::SmallPacket &payload, nite::IP_Port &ip)> lambda){
    for(int i = 0; i < deliveries.size(); ++i){
        auto pck = deliveries[i].get();
        if(pck->header == header){
            pck->onAck = lambda;
        }
    }
}