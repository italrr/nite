#include "Server.hpp"
#include "Network.hpp"

Server::Server(){
    running = false;
    genId = nite::randomInt(10, 20);
    isServer = true;
    setState(ST_WAITING_FOR_PLAYERS);
}

bool Server::listen(const nite::IP_Port &ip){
    if(running){
        nite::print("[server] server is already running. please close it down first.");
        return false;        
    }
    if(!socket.open(ip.port)){
        nite::print("[server] failed to start server");
        return false;
    }
    socket.setNonBlocking(true);
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
    sendPersPacketForMany(getAllClientsIps(), statech, getAllClientsAcks());
    for(int i = 0; i < onChangeCallback.size(); ++i){
        onChangeCallback[i](nstate, who, stateArg);
    }    
}

void Server::init(const Shared<GameState> &game){
    this->game = game;
    this->game->net = this;
}

void Server::clear(){
    socket.close();
    running = false;
    genId = nite::randomInt(10, 20);
    setState(ST_WAITING_FOR_PLAYERS);
}

void Server::notifyAllJoin(UInt8 id){
    auto &who = clients[id];
    nite::Packet join;
    join.setHeader(SV_CLIENT_JOIN);
    join.write(who->nickname);
    join.write(who->color.hex());
    join.write(&who->id, sizeof(who->id));
    join.write(&who->money, sizeof(who->money));
    sendPersPacketForMany(getAllClientsIps(), join, getAllClientsAcks());
}

void Server::update(){
    if(!running){
        return;
    }

    nite::IP_Port ip;
    nite::Packet buffer;
    if(socket.recv(ip, buffer) > 0){
        auto client = this->getClientByIp(ip);
        bool isLast = client.get() && buffer.getOrder() > client->lastRecvOrder;        
        if(client && isLast){
            client->lastRecvOrder = buffer.getOrder();
        }
        switch(buffer.getHeader()){
            case SV_CONNECT: {
                String nickname, color;
                buffer.read(nickname);
                buffer.read(color);
                // TODO: handle nicks
                // add player
                auto player = std::make_shared<ClientProxy>(ClientProxy());
                player->ip = ip;
                player->color = nite::Color(color);
                player->nickname = nickname;
                player->id = ++genId;
                player->order = this->clients.size();
                this->clients[player->id] = player;
                client = player;
                game->setupPlayer(client);
                // respond with accept
                nite::Packet accept;
                accept.setHeader(SV_CONNECT_ACCEPT);
                accept.write(client->nickname);
                accept.write(&client->id, sizeof(client->id));
                accept.write(game->gType->name);
                UInt8 nplayers = this->clients.size();;
                accept.write(&nplayers, sizeof(nplayers));
                for(auto &it : this->clients){
                    accept.write(it.second->nickname);
                    accept.write(it.second->color.hex());
                    accept.write(&it.second->id, sizeof(it.second->id));
                    accept.write(&it.second->money, sizeof(it.second->money));
                }
                sendPersPacketFor(ip, accept, ++client->svAck);
                // notify everyone someone joined
                notifyAllJoin(client->id);
                nite::print("Player '"+client->nickname+"' just joined");
            } break;
            case SV_PING: {
                if(!client || !isLast){
                    break;
                }
                nite::Packet pong;
                pong.setHeader(SV_PONG);
                pong.setOrder(++client->lastSentOrder);
                socket.send(client->ip, pong);    
            };
            case SV_PONG: {
                if(!client || !isLast){
                    break;
                }                
                client->lastPing = nite::getTicks();
            }; 
            default: {
                rcvPackets.push_back(buffer);
            };            
        }        

    }
    Vector<nite::IP_Port> timedoutClients;
    for(auto &it : clients){
        auto &client = it.second;
        if(nite::getTicks()-client->lastPing > 5000){
            timedoutClients.push_back(client->ip);
        }
    }
    for(int i = 0; i < timedoutClients.size(); ++i){
        // dropClient(timedoutClients[i], "timeout");
        nite::print("[server] dropped clientId "+timedoutClients[i].str()+": timeout");
    }
    // ping
    for(auto &it : clients){
        auto &client = it.second;
        if(nite::getTicks()-client->lastSentPing > 1000){
            client->lastSentPing = nite::getTicks();
            nite::Packet ping;
            ping.setHeader(SV_PING);
            ping.setOrder(++client->lastSentOrder);
            socket.send(client->ip, ping);
        }
    }    
    // process packets
    processIncomPackets();
    // deliver queue
    deliverPacketQueue();
    // update
    game->update();
}

void Server::processIncomPackets(){
    std::function<void(nite::Packet &handler, bool ignoreOrder)> processPacket = [&](nite::Packet &buffer, bool ignoreOrder){
        auto sender = buffer.sender;
        auto client = this->getClientByIp(sender);
        bool isLast = client && buffer.getOrder() > client->lastRecvOrder;
        if(client && isLast && !ignoreOrder){
            // client->lastPacketTimeout = nite::getTicks();
            // client->lastPacket = handler;
            client->lastRecvOrder = buffer.getOrder();
        }            
        // for multi-part packets
        if(ignoreOrder){
            isLast = true;
        }
        switch(buffer.getHeader()){
            /*
                SV_MULTI_PART_PACKET    
            */
            case SV_MULTI_PART_PACKET: {
                if(!client){
                    break;
                }                    
                Vector<UInt16> sizes;
                UInt8 total;
                buffer.read(&total, sizeof(total));
                for(int i = 0; i < total; ++i){
                    UInt16 size;
                    buffer.read(&size, sizeof(size));
                    sizes.push_back(size);
                }
                for(int i = 0; i < total; ++i){
                    nite::Packet holder;
                    buffer.read(holder.data, sizes[i]);
                    holder.sender = client->ip;
                    processPacket(holder, true);
                }
            } break;
            /*
                SV_ACK
            */            
            case SV_ACK: {
                ack(buffer);
            } break;  
            /*
                SV_SHUFFLE_DICE
            */
           case SV_SHUFFLE_DICE:{
               sendAck(sender, buffer.getAck());
           } break;                       

        }
        onAck(client->id, buffer);
    };
    for(int i = 0; i < rcvPackets.size(); ++i){
        processPacket(rcvPackets[i], false);
    }
    rcvPackets.clear();       
}

void Server::deliverPacketQueue(){
    updateDeliveries();
    if(packetQueue.size() < 1){
        return;
    }

    // deliver messages
    for(auto &it : clients){
        // packets will be joined together into bigger packets 
        auto &client = it.second;
        Vector<Vector<int>> indexes;
        Vector<Vector<UInt16>> sizes;
        static const size_t indexSize = sizeof(UInt16);
        size_t size;
        auto reset = [&](){
            indexes.push_back(Vector<int>());
            sizes.push_back(Vector<UInt16>());
            size = nite::NetworkMaxHeaderSize;
        };
        reset();
        // 1. calculate how many bigger packets are needed
        for(int i = 0; i < packetQueue.size(); ++i){
            auto &packet = packetQueue[i];
            // reset
            if(size + packet.maxSize > nite::NetworkMaxPacketSize){
                reset();                
            }
            size += packet.maxSize + sizeof(indexSize);
            indexes[indexes.size()-1].push_back(i);
            sizes[sizes.size()-1].push_back(packet.maxSize);
        }
        // 2. pack it up and send
        for(int i = 0; i < indexes.size(); ++i){
            nite::Packet handle(SV_MULTI_PART_PACKET);
            auto &index = indexes[i];
            auto &size = sizes[i];
            // write sizes
            UInt8 n = size.size();
            handle.write(&n, sizeof(n));
            for(int j = 0; j < size.size(); ++j){
                handle.write(&size[j], sizeof(size[j]));
            }
            // write packets
            for(int j = 0; j < index.size(); ++j){
                auto &packet = packetQueue[index[j]];
                handle.write(packet.data, packet.maxSize);
            }
            handle.setOrder(++client->lastSentOrder);
            socket.send(client->ip, handle);
        }
    }
    packetQueue.clear(); 
}