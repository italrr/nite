#include "Server.hpp"
#include "../Engine/Tools/Tools.hpp"

#include "Input.hpp"
#include "Object.hpp"
#include "../Entity.hpp"
#include "../Player.hpp"

Game::Server::Server(){
    this->init = false;
}

Game::Server::~Server(){

}

void Game::Server::preinit(){
    nite::print("looking for tileset sources...");
    Vector<String> tilesets;
    String path = "./data/tileset/";
    nite::fileFind(path, nite::Find::File, ".json", false, false, tilesets);
    for(int i = 0; i < tilesets.size(); ++i){
        Game::SvTilesetSource ts;
        String fullpath = path+tilesets[i];
        ts.hash = nite::hashFile(fullpath);
        ts.path = fullpath;
        ts.size = nite::fileSize(fullpath);
        this->tilesets[ts.hash] = ts;
    }
    nite::print("found "+nite::toStr(tilesets.size())+" tileset source(s)");
}

void Game::Server::listen(const String &name, UInt8 maxClients, UInt16 port){
    if(maxClients == 0){
        maxClients = 4;
    }
    this->name = name;
    this->maxClients = maxClients;
    if(!sock.open(nite::NetworkDefaultPort)){
        nite::print("[server] failed to start server");
        return;
    }
    sock.setNonBlocking(true);
    nite::print("[server] started server '"+name+"' | max clients "+nite::toStr(this->maxClients)+" | listening at "+nite::toStr(port));
    nite::print("[server] waiting for clients...");
    this->init = true;
}

Game::SvClient *Game::Server::getClient(UInt64 uid){
    if(clients.count(uid) > 0){
        return &clients[uid];
    }
    return NULL;
}

Game::SvClient *Game::Server::getClient(const String &nick){
    for(auto _cl : clients){
        if(_cl.second.nickname == nick){
            return &clients[_cl.first];
        }
    }
    return NULL;
}

void Game::Server::sendInfoPlayerList(UInt64 uid){
    auto cl = getClient(uid);
    if(cl == NULL){
        nite::print("failed to send player list to client '"+nite::toStr(uid)+"': it's not connected");
        return;
    }
    UInt16 total = clients.size();
    nite::Packet info(++cl->svOrder);
    info.setHeader(Game::PacketType::SV_CLIENT_LIST);
    info.write(&total, sizeof(UInt16));
    for(auto cl : clients){
        info.write(&cl.second.clientId, sizeof(UInt64));
        info.write(&cl.second.ping, sizeof(UInt64));
        info.write(cl.second.nickname);
    }
    persSend(cl->cl, info, 750, -1);
}

void Game::Server::dropClient(UInt64 uid, String reason){
    auto cl = this->getClient(uid);
    if(cl == NULL){
        nite::print("[server] cannot drop client "+nite::toStr(uid)+": it's not connected");
        return;
    }
    nite::Packet drop(++cl->lastSentOrder);
    drop.setHeader(Game::PacketType::SV_CLIENT_DROP);
    drop.write(reason);
    sock.send(cl->cl, drop);
    removeClient(uid);
    // notify client dropped to others
    nite::Packet noti(++cl->svOrder);
    noti.setHeader(Game::PacketType::SV_NOTI_CLIENT_DROP);
    noti.write(&uid, sizeof(UInt64));
    noti.write(reason);
    persSendAll(noti, 750, -1);
}

void Game::Server::dropClient(UInt64 uid){
    dropClient(uid, "no reason");
}

void Game::Server::removeClient(UInt64 uid){
    dropPersFor(uid);
    auto client = getClient(uid);
    if(!client){
        return;
    }
    auto it = clients.find(uid);
    clients.erase(it);
}

void Game::Server::close(){
    if(!init){
        return;
    }
    broadcast("server is closing down");
    nite::print("[server] closing server down...");
    nite::print("[server] dropping clients...");
    Vector<UInt32> ids;
    for(auto cl : clients){
        ids.push_back(cl.first);
    }
    for(int i = 0; i < ids.size(); ++i){
        dropClient(ids[i], "server closing down");
    }
    clear();
    broadcast("[server] closed server down");
}

void Game::Server::clear(){
    this->init = false;
    sock.close();
    deliveries.clear();
    clients.clear();
    players.clear();
    maps.clear();
    tilesets.clear();
    world.clear();
}

void Game::Server::update(){
    if(!init){
        return;
    }
    nite::IP_Port sender;
    nite::Packet handler;
    if(sock.recv(sender, handler)){
        UInt64 netId = sender.address+sender.port+sock.getSock();
        auto client = this->getClient(netId);
        bool isLast = client && handler.getOrder() > client->lastRecvOrder;
        if(client && isLast){
            client->lastPacketTimeout = nite::getTicks();
            client->lastPacket = handler;
            client->lastRecvOrder = handler.getOrder();
        }
        switch(handler.getHeader()){
            /*
                SV_ACK
            */            
            case Game::PacketType::SV_ACK: {
                ack(handler);
            } break;
            /*
                SV_PONG
            */      
            case Game::PacketType::SV_PONG: {
                if(!client){
                    break;
                }
                client->ping = nite::getTicks() - client->lastPing; 
            } break; 
            /*
                SV_CLIENT_DISCONNECT
            */             
            case Game::PacketType::SV_CLIENT_DISCONNECT: {
                if(!client){
                    break;
                }
                String reason;
                handler.read(reason);
                if(reason.length() == 0){
                    reason = "no reason";
                }
                nite::print("[server] disconnected clientId "+nite::toStr(netId)+": "+reason);
                removeClient(netId);   
                // notify client dropped to others
                nite::Packet noti;
                noti.setHeader(Game::PacketType::SV_NOTI_CLIENT_DROP);
                noti.write(&netId, sizeof(UInt64));
                noti.write(reason);
                persSendAll(noti, 750, -1);                                      
            } break;                  
            /*
                SV_PING
            */             
            case Game::PacketType::SV_PING: {
                if(!client){
                    break;
                }
                nite::Packet pong(++client->lastSentOrder);
                pong.setHeader(Game::PacketType::SV_PONG);
                sock.send(client->cl, pong);                
            } break;                             
            /*
                SV_CONNECT_REQUEST
            */
            case Game::PacketType::SV_CONNECT_REQUEST : {
                nite::print("[server] incoming connection...");
                if(clients.size() == maxClients){
                    nite::Packet reject((UInt32)0);
                    reject.setHeader(Game::PacketType::SV_CONNECT_REJECT);
                    reject.write("server is full");
                    nite::print("[server] rejected client "+nite::toStr(netId)+" : server full");
                    sock.send(sender, reject); 
                    break;
                }
                UInt32 clVer;
                String nick;
                handler.read(&clVer, sizeof(UInt32));
                handler.read(nick);
                // TODO: handle repeated nicks
                if(client){
                    dropClient(netId, "client of same of origin");
                }else{
                    Game::SvClient cl;
                    cl.nickname = nick;
                    cl.clientId = netId;
                    cl.lastPacketTimeout = nite::getTicks();
                    cl.role = Game::SvClientRole::Player;
                    cl.cl = sender;
                    clients[netId] = cl;
                    nite::Packet accepted(++clients[netId].svOrder);
                    accepted.setHeader(Game::PacketType::SV_CONNECT_ACCEPT);
                    accepted.write(&netId, sizeof(UInt64));
                    accepted.write(this->name);
                    persSend(clients[netId].cl, accepted);
                    nite::print("[server] accepted clientId "+nite::toStr(netId)+" | nickname '"+nick+"'");
                    // notify clients someone joined
                    nite::AsyncTask::spawn(nite::AsyncTask::ALambda([&](nite::AsyncTask::Context &ct){
                        nite::Packet notify;
                        UInt64 *uid = static_cast<UInt64*>(ct.payload);
                        auto &cl = clients[*uid];
                        notify.setHeader(Game::PacketType::SV_CLIENT_JOIN);
                        notify.write(&cl.clientId, sizeof(UInt64));
                        notify.write(cl.nickname);
                        persSendAll(notify, 750, -1);     
                        sendInfoPlayerList(*uid);
                        delete uid;
                        ct.stop();                   
                    }), 100, new UInt64(netId));
                }
            } break; 
            /*
                SV_CHAT_MESSAGE
            */             
            case Game::PacketType::SV_CHAT_MESSAGE: {
                if(!client){
                    break;
                }
                sendAck(client->cl, handler.getOrder(), ++client->lastSentOrder);
                UInt64 uid;
                String msg;
                handler.read(&uid, sizeof(UInt64));
                handler.read(msg);
                nite::print("SERVER ["+client->nickname+"] "+msg);
                persSendAll(handler, 1500, 4);
            } break; 
            /*
                SV_CLIENT_INPUT
            */
            case Game::PacketType::SV_CLIENT_INPUT: {
                if(!client){
                    break;
                }
                UInt8 amount;
                Vector<Game::InputFrameBuffer> buffer;
                handler.read(&amount, sizeof(UInt8));
                for(int i = 0; i < amount; ++i){
                    UInt8 key, type, lastStroke;
                    handler.read(&key, sizeof(UInt8));
                    handler.read(&type, sizeof(UInt8));
                    handler.read(&lastStroke, sizeof(UInt8));
                    buffer.push_back(InputFrameBuffer(key, type, lastStroke));
                    client->input.update(buffer);
                }
            } break;                           
        }
    }

    // refresh player info
    if(nite::getTicks()-lastPlayerInfoSent > 1000){
        lastPlayerInfoSent = nite::getTicks();
        for(auto cl : clients){
            sendInfoPlayerList(cl.first);
        }        
    }

    // timeout
    Vector<UInt64> timedoutClients;
    for(auto cl : clients){
        if(nite::getTicks()-cl.second.lastPacketTimeout > ClientTimeout){
            timedoutClients.push_back(cl.first);
        }
    }
    for(int i = 0; i < timedoutClients.size(); ++i){
        dropClient(timedoutClients[i], "timeout");
        nite::print("[server] dropped clientId "+nite::toStr(timedoutClients[i])+": timeout");
    }

    // ping
    for(auto cl : clients){
        auto &client = clients[cl.first];
        if(nite::getTicks()-client.lastPing > 500){
            client.lastPing = nite::getTicks();
            nite::Packet ping(++client.lastSentOrder);
            ping.setHeader(Game::PacketType::SV_PING);
            sock.send(client.cl, ping);
        }
    }

    game();
    updateDeliveries();
}

void Game::Server::sendAll(nite::Packet packet){
    if(!init){
        return;
    }    
    for(auto cl : clients){
        nite::Packet cpy = packet;
        cpy.setOrder(++cl.second.lastSentOrder);
        sock.send(cl.second.cl, cpy);
    }
}

void Game::Server::persSendAll(nite::Packet packet, UInt64 timeout, int retries){
    if(!init){
        return;
    }    
    for(auto cl : clients){
        nite::Packet cpy = packet;
        cpy.setOrder(++cl.second.svOrder); // pers expect ack
        persSend(cl.second.cl, cpy, timeout, retries);
    }
}

void Game::Server::broadcast(const String &message){
    if(!init){
        return;
    }    
    if(message.length() == 0){
        nite::print("[server] cannot broadcast an empty message");
        return;
    }
    nite::Packet msg;
    msg.setHeader(Game::PacketType::SV_BROADCAST_MESSAGE);
    msg.write(message);
    nite::print("[BROADCAST] "+message);
    persSendAll(msg, 750, 4);
}

void Game::Server::game(){
    
}

void Game::Server::setupGame(const String &name, int maxClients, int maps){
    if(maps <= 0){
        maps = 1;
    }
    if(maxClients <= 0){
        maxClients = 4;
    }
    nite::print("[server] setting up game for "+nite::toStr(maxClients)+" players | "+nite::toStr(maps)+" maps");
    Game::RING::TileSource src("data/tileset/dungeon.json");
    // build maps
    for(int i = 0; i < maps; ++i){
        Shared<Game::RING::Blueprint> bp = Shared<Game::RING::Blueprint>(new Game::RING::Blueprint());
        Shared<Game::RING::Map> map = Shared<Game::RING::Map>(new Game::RING::Map());
        bp->generate(35, 35);
        map->build(bp, src, 3);
        this->maps.push_back(map);
    }
    listen(name, maxClients, nite::NetworkDefaultPort);
    // TODO: send map to the players
    
    // wait some time before starting the game here
    nite::print("starting game in 5 secs...");
    nite::AsyncTask::spawn(nite::AsyncTask::ALambda([&](nite::AsyncTask::Context &ct){
        for(auto cl : clients){
            createPlayer(cl.second.clientId, 1);
        }
        ct.stop();
    }), 5000);
}

Shared<Game::NetObject> Game::Server::spawn(Shared<Game::NetObject> obj){
    if(!init){
        return Shared<Game::NetObject>(NULL);
    }
    if(obj.get() == NULL){
        nite::print("cannot spawn undefined object");
        return Shared<Game::NetObject>(NULL);
    }    
    auto id = this->world.add(obj);
    nite::Packet crt;
    crt.setHeader(Game::PacketType::SV_CREATE_OBJECT);
    crt.write(&id, sizeof(UInt16));
    crt.write(&obj->sigId, sizeof(UInt16));
    crt.write(&obj->position.x, sizeof(float));
    crt.write(&obj->position.y, sizeof(float));
    persSendAll(crt, 1000, -1);
    return obj;
}

bool Game::Server::destroy(UInt32 id){
    if(!world.exists(id)){
        nite::print("failed to destroy object id '"+nite::toStr(id)+"': it doesn't exist");
        return false;
    }
    auto &obj = world.objects[id];
    nite::Packet des;
    des.setHeader(Game::PacketType::SV_DESTROY_OBJECT);
    des.write(&id, sizeof(UInt32));
    obj->destroy();
    persSendAll(des, 1000, -1);    
    return true;
}

Shared<Game::NetObject> Game::Server::createPlayer(UInt64 uid, UInt32 lv){
    auto client = clients.find(uid);
    if(client == clients.end()){
        nite::print("[server] failed to create player for unexisting client id '"+nite::toStr(uid)+"'");
        return Shared<Game::NetObject>(NULL);
    }
    auto ent = Shared<Game::NetObject>(new Game::Player());
    auto player = static_cast<Player*>(ent.get());
    player->startLv = lv;
    player->position.x = nite::randomInt(0, 100);
    player->position.y = nite::randomInt(0, 100);
    client->second.entityId = ent->id;
    players[uid] = ent->id;
    spawn(ent);
    nite::print("[server] created player entity with id "+nite::toStr(ent->id)+" | for client id "+nite::toStr(uid)+"("+client->second.nickname+")");
    return ent;
}

bool Game::Server::removePlayer(UInt64 uid){
    auto client = clients.find(uid);
    if(client == clients.end()){
        nite::print("failed to remove player for unexisting client id '"+nite::toStr(uid)+"'");
        return false;
    }
    auto entId = client->second.entityId;
    auto player = players.find(uid);
    if(!world.exists(entId) || entId == 0){
        nite::print("failed to remove player with id "+nite::toStr(uid)+": it doesn't exist");
        return false;
    }
    if(player != players.end()){
        players.erase(player);
    }
    destroy(entId);
    return true;
}

bool Game::Server::killPlayer(UInt64 uid){
    return false;
}