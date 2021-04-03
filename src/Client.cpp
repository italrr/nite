#include "Client.hpp"

Client::Client(){
    connected = false;
    id = 0;
    lastSentPing = nite::getTicks();
    lastPing = nite::getTicks();
    svAck = 0;
    sentOrder = 1;
    rcvOrder = 0;    
    setState(ST_WAITING_FOR_PLAYERS);
}

bool Client::connect(const nite::IP_Port &ip){
    if(connected){
        nite::print("[client] client is currently connected to a server. please disconnect first.");
        return false;
    }
    if(!socket.open(ip.port + nite::randomInt(1, 200))){
        nite::print("[client] failed to open UDP Socket. cannot connect to "+ip.str());
        return false;
    }
    socket.setNonBlocking(true);
    this->sv = ip;
    nite::Packet connect;
    connect.setHeader(SV_CONNECT);
    connect.write(nickname);
    connect.write(color.hex());
    socket.send(ip, connect);
    connected = true;
    setState(ST_WAITING_FOR_PLAYERS);
    return true;
}

void Client::init(const Shared<GameState> &game, const String &nickname, const nite::Color &color){
    this->nickname = nickname;
    this->game = game;
    this->game->net = this;
    this->color = color;
}

void Client::clear(){
    setState(ST_WAITING_FOR_PLAYERS);
    connected = false;
    socket.close();
    id = 0;
    svAck = 0;
    sentOrder = 1;
    rcvOrder = 0;     
}

void Client::update(){
    game->game();
    if(!connected){
        return;
    }

    nite::Packet buffer;
    nite::IP_Port ip;
    int nb = socket.recv(ip, buffer);
    if(nb > 0){ 
        bool isLast = buffer.getOrder() > rcvOrder; 
        bool isSv = true;       
        switch(buffer.getHeader()){
            case SV_CONNECT_REJECT: {
                if(!isSv){ break; }
                String reason;
                buffer.read(reason);
                nite::print("connection rejected: "+reason);
                clear();
                return;
            } break;
            // TODO: Client drop
            case SV_PING: {
                 if(!isSv || !isLast){ break; }
                nite::Packet pong;
                pong.setHeader(SV_PONG);
                pong.setOrder(++sentOrder);
                socket.send(this->sv, pong);                    
            };            
            case SV_PONG: {
                if(!isSv || !isLast){ break; }
                this->lastPing = nite::getTicks();
            } break;
            default: {
                rcvPackets.push_back(buffer);
            } break;                 
        }  
    }

    if(nite::getTicks()-lastPing > 5000){
        nite::print("[client] disconnected from "+sv.str()+": timeout");
        clear();
        return;
    }

    if(nite::getTicks()-lastSentPing > 1000){
        lastSentPing = nite::getTicks();
        nite::Packet ping;
        ping.setHeader(SV_PING);
        ping.setOrder(++sentOrder);
        socket.send(this->sv, ping);        
    }
    // process packets
    processIncomPackets();
    // deliver queue
    deliverPacketQueue();    
    game->update();
}


void Client::processIncomPackets(){
    std::function<void(nite::Packet &handler, bool ignoreOrder)> processPacket = [&](nite::Packet &buffer, bool ignoreOrder){
        auto sender = buffer.sender;
        bool isLast = buffer.getOrder() > rcvOrder;
        bool isSv = true;
        if(isLast && !ignoreOrder){
            // lastPacket = handler;
            // lastPacketTimeout = nite::getTicks();
            rcvOrder = buffer.getOrder();
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
                    holder.sender = buffer.sender;
                    processPacket(holder, true);
                }
            } break; 

            /*
                SV_CONNECT_ACCEPT
            */
            case SV_CONNECT_ACCEPT: {
                sendAck(sender, buffer.getAck());
                String gameType;
                UInt8 players;
                buffer.read(this->nickname);
                buffer.read(&this->id, sizeof(this->id));
                buffer.read(gameType);
                buffer.read(&players, sizeof(players));
                for(int i = 0; i < players; ++i){
                    auto client = std::make_shared<ClientProxy>(ClientProxy());
                    buffer.read(client->nickname);
                    String color;
                    buffer.read(color);
                    client->color.set(color);                    
                    buffer.read(&client->id, sizeof(client->id));
                    buffer.read(&client->money, sizeof(client->money));
                    this->clients[client->id] = client;
                }
                nite::print("Joined game. your nick is '"+nickname+"'("+nite::toStr(this->id)+")");
            } break;                               
            /*
                SV_ACK
            */
            case SV_ACK: {
                ack(buffer);
            } break;
            /*
                SV_CLIENT_JOIN
            */
            case SV_CLIENT_JOIN: {
                if(!isSv || !isLast){ break; }
                auto client = std::make_shared<ClientProxy>(ClientProxy());
                buffer.read(client->nickname);
                String color;
                buffer.read(color);
                client->color.set(color);
                buffer.read(&client->id, sizeof(client->id));
                buffer.read(&client->money, sizeof(client->money));
                if(client->id == this->id){
                    break;
                }
                nite::print(client->nickname+" joined the game");
                this->clients[client->id] = client;

                sendAck(sender, buffer.getAck());
            } break;
            /*
                SV_GAME_STATE
            */            
            case SV_GAME_STATE: {
                if(!isSv || !isLast){ break; }
                UInt8 currentState, who;
                Int32 statearg;
                buffer.read(&currentState, sizeof(currentState));
                buffer.read(&who, sizeof(who));
                buffer.read(&statearg, sizeof(statearg));
                setState(currentState, who, statearg);
                this->lastStateArg = statearg;
                this->lastStateWho = who;
                nite::print("server changed state: "+nite::toStr(currentState));
                sendAck(sender, buffer.getAck());
            } break;  
            /*
                SV_SHUFFLE_DICE
            */
            case SV_SHUFFLE_DICE: {
                sendAck(sender, buffer.getAck());

            } break;
            /*
                SV_ADVANCE_PLAYER_TO
            */
            case SV_ADVANCE_PLAYER_TO: {
                sendAck(sender, buffer.getAck());
            } break;           
            /*
                SV_BROADCAST_NOTIFICATION
            */
            case SV_BROADCAST_NOTIFICATION: {
                sendAck(sender, buffer.getAck());
            } break;  
            /*
                SV_PLAYER_TURN_ORDER_LIST
            */               
            case SV_PLAYER_TURN_ORDER_LIST: {
                sendAck(sender, buffer.getAck());
            } break;
            /*
                Unknown
            */
            default: {
                if(!isSv){ break; }
                nite::print("[client] unknown packet type '"+nite::toStr(buffer.getHeader())+"'");
            } break;
        }
        onAck(0, buffer);
        // onAck()
    };

    for(int i = 0; i < rcvPackets.size(); ++i){
        processPacket(rcvPackets[i], false);
    }

    rcvPackets.clear();
}

void Client::deliverPacketQueue(){
    updateDeliveries();
    // deliver messages
    if(packetQueue.size() < 1){
        return;
    }        
    // packets will be joined together into bigger packets 
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
        handle.setOrder(++sentOrder);
        socket.send(sv, handle);
    }
    packetQueue.clear();
}

void Client::draw(){
    // draw
    nite::setRenderTarget(nite::RenderTargetGame);
    game->board->draw();
    game->dice->draw(game->board);
}