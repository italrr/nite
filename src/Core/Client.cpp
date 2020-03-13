#include "../Engine/Tools/Tools.hpp"
#include "../Engine/Graphics.hpp"
#include "../Engine/Console.hpp"
#include "Client.hpp"
#include "../Entity/Base.hpp"
#include "../Game.hpp"

static void pipeServerSideCmds(const String &input){
    auto core = Game::getGameCoreInstance();
    if(core == NULL){
        return;
    }
    auto &cl = core->client;
    nite::Packet cmd(++cl.svOrder);
    cmd.setHeader(Game::PacketType::SV_REMOTE_CMD_EXEC);
    cmd.write(input);
    cl.persSend(cl.sv, cmd, 1000, -1);
}

/*
    RCON PWD
*/
static nite::Console::Result cfRcon(Vector<String> params){
    auto core = Game::getGameCoreInstance();
    if(core == NULL){
        return nite::Console::Result("fatal error: core was not init", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
    }
    auto &cl = core->client;    
    if(params.size() == 0){
        return nite::Console::Result("not enough parameters(1)", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
    }
    auto pwd = params[0];
    String hash = nite::hashString(pwd);
    nite::Packet rcon(++cl.svOrder);
    rcon.setHeader(Game::PacketType::SV_RCON);
    rcon.write(hash);
    cl.persSend(cl.sv, rcon, 1000, -1);
    return nite::Console::Result();
}
static auto cfRconIns = nite::Console::CreateFunction("rcon", &cfRcon);

/*
    CONNECT IP PORT(OPT)
*/
static nite::Console::Result cfConnect(Vector<String> params){
    auto core = Game::getGameCoreInstance();
    if(core == NULL){
        return nite::Console::Result("fatal error: core was not init", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
    }
    auto &cl = core->client;    
    if(params.size() < 1){
        return nite::Console::Result("not enough parameters(1)", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
    }
    auto ip = params[0];
    auto port = params.size() >= 2 && nite::isNumber(params[1]) ? nite::toInt(params[1]) : nite::NetworkDefaultPort;
    cl.connect(ip, port);
    return nite::Console::Result();
}
static auto cfConnectIns = nite::Console::CreateFunction("connect", &cfConnect);

/*
    DISCONNECT
*/
static nite::Console::Result cfDisconnect(Vector<String> params){
    auto core = Game::getGameCoreInstance();
    if(core == NULL){
        return nite::Console::Result("fatal error: core was not init", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
    }
    String reason = params.size() > 0 ? params[0] : "disconnected by user";
    auto &cl = core->client;    
    if(cl.state != Game::NetState::Connected){
        return nite::Console::Result("must be connected to a server before disconnecting", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
    }
    cl.disconnect(reason);
    return nite::Console::Result();
}
static auto cfDisconnectIns = nite::Console::CreateFunction("disconnect", &cfDisconnect);

Game::Client::Client() : Game::Net(){
    init = false;
    clear();
}

Game::Client::~Client(){

}

void Game::Client::clear(){
    svOrder = 0;
    sentOrder = 1;
    rcvOrder = 0;
    serverId = 0;
    entityId = 0;
    connected = false;
    sock.close();
    clients.clear();
    world.clear();
    deliveries.clear();
    hud.stop();
    setState(Game::NetState::Disconnected); 
}

void Game::Client::connect(const String &ip, UInt16 port){
    if(connected){
        nite::print("[client] disconnect from current server");
        return;
    }
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
    nite::Console::pipeServerSideExecs(&pipeServerSideCmds);
    lastPacketTimeout = nite::getTicks();
    sock.setNonBlocking(true);
    nite::Packet connect;
    connect.setOrder(0);
    connect.setHeader(Game::PacketType::SV_CONNECT_REQUEST);
    connect.write((char*)&Game::CLIENT_VERSION, sizeof(UInt32));
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
    this->icons.load(nite::Texture("data/ui/icons/test_icons.png"), nite::Vec2(32.0f));
}

void Game::Client::disconnect(){
    disconnect("");
}

void Game::Client::disconnect(const String &reason){
    if(!init || state == Game::NetState::Disconnected){
        return;
    }
    nite::Packet disc(++sentOrder);
    disc.setHeader(Game::PacketType::SV_CLIENT_DISCONNECT);
    disc.write(reason);
    sock.send(sv, disc);
    nite::print("[client] disconnected from "+sv.str());
    clear();
}

void Game::Client::sendChatMsg(const String &msg){
    if(!init || state != Game::NetState::Connected){
        return;
    }
    nite::Packet pack(++svOrder);
    pack.setHeader(Game::PacketType::SV_CHAT_MESSAGE);
    pack.write(&clientId, sizeof(UInt64));
    pack.write(msg);
    persSend(sv, pack, 1500, 4);
}

void Game::Client::update(){
    // exit if no connection
    if(state == Game::NetState::Disconnected || !init){
        return;
    }
    nite::Packet handler;
    nite::IP_Port sender;
    if(sock.recv(sender, handler)){
        UInt64 netId = sender.address + sender.port + sock.getSock();
        bool isSv = netId == this->serverId && state != Game::NetState::Disconnected;
        bool isLast = isSv && handler.getOrder() > rcvOrder;
        if(isSv && isLast){
            lastPacket = handler;
            lastPacketTimeout = nite::getTicks();
            rcvOrder = handler.getOrder();
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
                if(!isSv){ break; }  
                this->ping = nite::getTicks() - this->lastPing;           
            } break;             
            /*
                SV_PING
            */             
            case Game::PacketType::SV_PING: {
                if(!isSv){ break; }     
                nite::Packet pong(++sentOrder);
                pong.setHeader(Game::PacketType::SV_PONG);
                sock.send(this->sv, pong);                
            } break;
            /*
                SV_CLIENT_DROP
            */             
            case Game::PacketType::SV_CLIENT_DROP: {
                if(!isSv){ break; }  
                String reason;
                handler.read(reason);
                if(reason.length() == 0){
                    reason = "no reason";
                }
                nite::print("[client] server dropped connection: "+reason);
                clear();               
            } break;                        
            /*
                SV_CONNECT_REJECT
            */             
            case Game::PacketType::SV_CONNECT_REJECT: {
                if(state != Game::NetState::Connecting){
                    break;
                }
                String reason;
                handler.read(reason);
                nite::print("[client] connection rejected: "+reason);
                clear();             
            } break;
            /*
                SV_CONNECT_ACCEPT
            */             
            case Game::PacketType::SV_CONNECT_ACCEPT: {
                if(state == Game::NetState::Connecting){
                    this->sv = sender;
                    this->serverId = netId;
                    this->connected = true;
                    handler.read(&clientId, sizeof(UInt64));
                    handler.read(serverName);
                    Game::ClClient yourself;
                    yourself.uid = clientId;
                    yourself.nickname = nickname;
                    this->clients[clientId] = yourself;
                    setState(Game::NetState::Connected);
                    nite::print("[client] connected to "+this->sv.str()+" | clientId "+nite::toStr(this->clientId));                    
                }
                sendAck(sender, handler.getOrder(), ++sentOrder);
            } break;
            /*
                SV_CLIENT_JOIN
            */
            case Game::PacketType::SV_CLIENT_JOIN: {
                if(!isSv){ break; }             
                UInt64 uid;
                String nick;
                handler.read(&uid, sizeof(UInt64));
                handler.read(nick);
                this->clients[uid] = Game::ClClient(uid, nick);
                nite::print(nick+" joined the game");
                // TODO: add in-game notification for this message
                sendAck(sender, handler.getOrder(), ++sentOrder);
            } break;
            /*
                SV_NOTI_CLIENT_DROP
            */
            case Game::PacketType::SV_NOTI_CLIENT_DROP: {
                if(!isSv){ break; }               
                UInt64 uid;
                String reason;
                handler.read(&uid, sizeof(UInt64));
                handler.read(reason);
                auto it = clients.find(uid);
                if(it != clients.end()){
                    nite::print(it->second.nickname+" left the game: "+reason);
                    clients.erase(it);
                }
                // TODO: add in-game notification for this message
                sendAck(sender, handler.getOrder(), ++sentOrder);
            } break;            
            /*
                SV_CLIENT_LIST
            */
            case Game::PacketType::SV_CLIENT_LIST: {
                if(!isSv){ break; }                
                clients.clear();
                UInt16 n;
                handler.read(&n, sizeof(UInt16));
                for(int i = 0; i < n; ++i){
                    Game::ClClient player;           
                    handler.read(&player.uid, sizeof(UInt64));
                    handler.read(&player.ping, sizeof(UInt64));
                    handler.read(player.nickname);  
                    clients[player.uid] = player;
                }
                sendAck(this->sv, handler.getOrder(), ++sentOrder);
            } break;
            /*
                SV_BROADCAST_MESSAGE
            */
            case Game::PacketType::SV_BROADCAST_MESSAGE: {
                if(!isSv){ break; }              
                String msg;
                handler.read(msg);
                nite::print(msg);
                // TODO: add in-game notification for this message (and chat)
                sendAck(this->sv, handler.getOrder(), ++sentOrder);
            } break;   
            /*
                SV_CHAT_MESSAGE
            */
           case Game::PacketType::SV_CHAT_MESSAGE: {
                if(!isSv){ break; }               
                UInt64 uid;
                String msg;
                handler.read(&uid, sizeof(UInt64));
                handler.read(msg);
                auto it = clients.find(uid);
                String who = it != clients.end() ? it->second.nickname : "???";
                nite::print(this->nickname+" ["+who+"] "+msg);
                // TODO: add in-game chat
                sendAck(this->sv, handler.getOrder(), ++sentOrder);
           } break; 

            /*
                SV_CREATE_OBJECT
            */
            case Game::PacketType::SV_CREATE_OBJECT: {
                if(!isSv){ break; }      
                UInt16 id;
                UInt16 sigId;
                float x, y;
                sendAck(this->sv, handler.getOrder(), ++sentOrder);
                handler.read(&id, sizeof(UInt16));
                handler.read(&sigId, sizeof(Int16));
                handler.read(&x, sizeof(float));
                handler.read(&y, sizeof(float));
                auto obj = createNetObject(id, sigId, x, y);
                if(obj.get() == NULL){
                    nite::print("[client] fail SV_CREATE_OBJECT: undefined obj sig '"+Game::ObjectSig::name(sigId)+"' on the client");
                    break;
                }
                if(world.objects.find(id) != world.objects.end()){
                    nite::print("[client] fail SV_CREATE_OBJECT: duplicated id "+nite::toStr(id)+"");
                    // TODO: come up with a way to properly handle duplicated ids?
                    break;
                }
                obj->onCreate();
                obj->readInitialStateForSync(handler);
                world.objects[id] = obj;
                nite::print("[client] spawned object: '"+Game::ObjectSig::name(sigId)+"' id: "+nite::toStr(id)+", type: '"+Game::ObjectType::name(obj->objType)+"', sigId: "+Game::ObjectSig::name(sigId)+" at "+nite::Vec2(x, y).str());
            } break;
            /*
                SV_DESTROY_OBJECT
            */
            case Game::PacketType::SV_DESTROY_OBJECT: {
                if(!isSv){ break; }  
                UInt16 id;
                handler.read(&id, sizeof(UInt16));
                sendAck(this->sv, handler.getOrder(), ++sentOrder);
                auto obj = world.objects.find(id);
                if(obj == world.objects.end()){
                    nite::print("[client] fail SV_DESTROY_OBJECT: object id "+nite::toStr(id)+" doesn't exist");
                    break;
                }
                world.objects.erase(obj);
            } break;
            /*
                SV_UPDATE_PHYSICS_OBJECT
            */
            case Game::PacketType::SV_UPDATE_PHYSICS_OBJECT: {
                if(!isSv){ break; }  
                UInt16 amnt;
                handler.read(&amnt, sizeof(UInt16));
                for(int i = 0; i < amnt; ++i){
                    UInt16 id;
                    float x, y, sx, sy;
                    handler.read(&id, sizeof(UInt16));
                    handler.read(&x, sizeof(float));
                    handler.read(&y, sizeof(float));
                    handler.read(&sx, sizeof(float));
                    handler.read(&sy, sizeof(float));                    
                    auto it = world.objects.find(id);
                    if(it != world.objects.end()){
                        it->second->position.set(x, y);
                        it->second->speed.set(sx, sy);
                    }
                }
            } break;  
            /*
                SV_UPDATE_WORLD_SIMULATION_PROPS
            */
            case Game::PacketType::SV_UPDATE_WORLD_SIMULATION_PROPS: {
                if(!isSv){ break; }  
                handler.read(&world.timescale, sizeof(float));
                sendAck(this->sv, handler.getOrder(), ++sentOrder);
            } break;              
            /*
                SV_UPDATE_OBJECT_RELATIVE_TIMESCALE
            */
            case Game::PacketType::SV_UPDATE_OBJECT_RELATIVE_TIMESCALE: {
                if(!isSv){ break; }  
                sendAck(this->sv, handler.getOrder(), ++sentOrder);
                UInt16 id;
                float timescale;
                handler.read(&id, sizeof(UInt16));
                handler.read(&timescale, sizeof(float));
                auto it = world.objects.find(id);
                if(it != world.objects.end()){
                    it->second->relativeTimescale = timescale;
                }
            } break;  
            /*
                SV_REMOTE_CMD_MSG
            */                          
            case Game::PacketType::SV_REMOTE_CMD_MSG: {
                if(!isSv){ break; }  
                sendAck(this->sv, handler.getOrder(), ++sentOrder);
                String msg;
                nite::Color color;
                handler.read(msg);
                handler.read(&color.r, sizeof(float));
                handler.read(&color.g, sizeof(float));
                handler.read(&color.b, sizeof(float));
                nite::Console::add(msg, color);
            } break;      
            /*
                SV_SET_GAME_START
            */          
            case Game::PacketType::SV_SET_GAME_START: {
                if(!isSv){ break; }  
                sendAck(this->sv, handler.getOrder(), ++sentOrder);
                this->onStart();
            } break;       
            /*
                SV_NOTI_ENTITY_OWNER
            */          
            case Game::PacketType::SV_NOTI_ENTITY_OWNER: {
                if(!isSv){ break; }  
                sendAck(this->sv, handler.getOrder(), ++sentOrder);
                handler.read(&this->entityId, sizeof(UInt16));
                this->hud.setFollow(this->entityId);
            } break;       
                        
            /*
                SV_SET_ENTITY_SKILLS
            */          
            case Game::PacketType::SV_SET_ENTITY_SKILLS: {
                if(!isSv){ break; }  
                sendAck(this->sv, handler.getOrder(), ++sentOrder);
                UInt16 entId;
                UInt8 amnt;
                handler.read(&entId, sizeof(UInt16));
                handler.read(&amnt, sizeof(UInt8));
                auto it = world.objects.find(entityId);
                if(it == world.objects.end()){
                    nite::print("[client] fail SV_SET_ENTITY_SKILLS: entity id doesn't exist");
                    break;
                }
                for(int i = 0; i < amnt; ++i){
                    UInt16 skId;
                    UInt8 lv;
                    handler.read(&skId, sizeof(UInt16));
                    handler.read(&lv, sizeof(UInt8));
                    if(getSkill(skId, lv).get() == NULL){
                        nite::print("[client] warn SV_SET_ENTITY_SKILLS: skill id "+nite::toStr(skId)+" doesn't exist");
                        continue;
                    }
                    auto ent = static_cast<Game::EntityBase*>(it->second.get());
                    ent->skillStat.add(skId, lv);
                }
            } break; 
            /*
                SV_ADD_ENTITY_SKILL
            */ 
            case Game::PacketType::SV_ADD_ENTITY_SKILL: {
                if(!isSv){ break; }  
                sendAck(this->sv, handler.getOrder(), ++sentOrder);
                UInt16 entId, skId;
                UInt8 lv;
                handler.read(&entId, sizeof(UInt16));
                handler.read(&skId, sizeof(UInt16));
                handler.read(&lv, sizeof(UInt8));
                auto it = world.objects.find(entityId);
                if(it == world.objects.end()){
                    nite::print("[client] fail SV_ADD_ENTITY_SKILL: entity id doesn't exist");
                    break;
                }      
                if(getSkill(skId, 0).get() == NULL){
                    nite::print("[client] warn SV_ADD_ENTITY_SKILL: skill id "+nite::toStr(skId)+" doesn't exist");
                    break;
                }
                auto ent = static_cast<Game::EntityBase*>(it->second.get());
                ent->skillStat.add(skId, lv);                          
            } break;            
            /*
                SV_REMOVE_ENTITY_SKILLS
            */          
            case Game::PacketType::SV_REMOVE_ENTITY_SKILLS: {
                if(!isSv){ break; }  
                sendAck(this->sv, handler.getOrder(), ++sentOrder);
                UInt16 entId;
                UInt8 amnt;
                handler.read(&entId, sizeof(UInt16));
                handler.read(&amnt, sizeof(UInt8));
                auto it = world.objects.find(entityId);
                if(it == world.objects.end()){
                    nite::print("[client] fail SV_REMOVE_ENTITY_SKILLS: entity id doesn't exist");
                    break;
                }
                for(int i = 0; i < amnt; ++i){
                    UInt16 skId;
                    handler.read(&skId, sizeof(UInt16));
                    if(getSkill(skId, 0).get() == NULL){
                        nite::print("[client] fail SV_REMOVE_ENTITY_SKILLS: skill id "+nite::toStr(skId)+" doesn't exist");
                        continue;
                    }
                    auto ent = static_cast<Game::EntityBase*>(it->second.get()); // ok i know, i repeated it enough (if you don't know what, keep reading the project...)
                    ent->skillStat.remove(skId);
                }
            } break; 
            /*
                SV_REMOVE_ENTITY_SKILL
            */ 
            case Game::PacketType::SV_REMOVE_ENTITY_SKILL: {
                if(!isSv){ break; }  
                sendAck(this->sv, handler.getOrder(), ++sentOrder);
                UInt16 entId, skId;
                handler.read(&entId, sizeof(UInt16));
                handler.read(&skId, sizeof(UInt16));
                auto it = world.objects.find(entityId);
                if(it == world.objects.end()){
                    nite::print("[client] fail SV_REMOVE_ENTITY_SKILL: entity id doesn't exist");
                    break;
                }      
                if(getSkill(skId, 0).get() == NULL){
                    nite::print("[client] fail SV_REMOVE_ENTITY_SKILL: skill id "+nite::toStr(skId)+" doesn't exist");
                    break;
                }
                auto ent = static_cast<Game::EntityBase*>(it->second.get());
                ent->skillStat.remove(skId);                          
            } break;            
            /*
                SV_SET_ENTITY_ACTIONABLES
            */          
            case Game::PacketType::SV_SET_ENTITY_ACTIONABLES: {
                if(!isSv){ break; }  
                sendAck(this->sv, handler.getOrder(), ++sentOrder);
                UInt16 entId;
                UInt8 amnt;
                handler.read(&entId, sizeof(UInt16));
                handler.read(&amnt, sizeof(UInt8));
                auto it = world.objects.find(entityId);
                if(it == world.objects.end()){
                    nite::print("[client] fail SV_SET_ENTITY_ACTIONABLES: entity id doesn't exist");
                    break;
                }
                for(int i = 0; i < amnt; ++i){
                    auto ent = static_cast<Game::EntityBase*>(it->second.get()); // yeap
                    handler.read(&ent->actionables[i].type, sizeof(UInt8));
                    handler.read(&ent->actionables[i].id, sizeof(UInt16));
                }
            } break;  
            /*
                SV_ADD_EFFECT
            */ 
            case Game::PacketType::SV_ADD_EFFECT: {
                if(!isSv){ break; }  
                sendAck(this->sv, handler.getOrder(), ++sentOrder);
                UInt16 entId, type, insId;
                handler.read(&entId, sizeof(UInt16));
                handler.read(&type, sizeof(UInt16));
                handler.read(&insId, sizeof(UInt16));
                auto it = world.objects.find(entId);
                if(it == world.objects.end()){
                    nite::print("[client] fail SV_ADD_EFFECT: entity id doesn't exist");
                    break;
                }      
                auto eff = getEffect(type);
                if(eff.get() == NULL){
                    nite::print("[client] fail SV_ADD_EFFECT: effect type "+nite::toStr(type)+" doesn't exist");
                    break;
                }
                auto ent = static_cast<Game::EntityBase*>(it->second.get());
                ent->effectStat.add(eff, insId);
                eff->readState(handler);                         
            } break;  
            /*
                SV_REMOVE_EFFECT
            */ 
            case Game::PacketType::SV_REMOVE_EFFECT: {
                if(!isSv){ break; }  
                sendAck(this->sv, handler.getOrder(), ++sentOrder);
                UInt16 entId, insId;
                handler.read(&entId, sizeof(UInt16));
                handler.read(&insId, sizeof(UInt16));
                auto it = world.objects.find(entId);
                if(it == world.objects.end()){
                    nite::print("[client] fail SV_REMOVE_EFFECT: entity id doesn't exist");
                    break;
                }      
                auto ent = static_cast<Game::EntityBase*>(it->second.get());
                ent->effectStat.remove(insId);                         
            } break;     
            /*
                SV_UPDATE_EFFECT
            */ 
            case Game::PacketType::SV_UPDATE_EFFECT: {
                if(!isSv){ break; }  
                sendAck(this->sv, handler.getOrder(), ++sentOrder);
                UInt16 entId, type, insId;
                handler.read(&entId, sizeof(UInt16));
                handler.read(&insId, sizeof(UInt16));
                auto it = world.objects.find(entId);
                if(it == world.objects.end()){
                    nite::print("[client] fail SV_UPDATE_EFFECT: entity id doesn't exist");
                    break;
                }      
                auto ent = static_cast<Game::EntityBase*>(it->second.get());
                auto itef = ent->effectStat.effects.find(insId);
                if(itef == ent->effectStat.effects.end()){
                    nite::print("[client] fail SV_UPDATE_EFFECT: effect ins id doesn't exist");
                    break;
                }
                auto eff = ent->effectStat.effects[itef->first];
                eff->readState(handler);                         
            } break;
            /*
                SV_ADD_ITEM
            */ 
            case Game::PacketType::SV_ADD_ITEM: {
                if(!isSv){ break; }  
                sendAck(this->sv, handler.getOrder(), ++sentOrder);
                UInt16 entId, itemId, slotId, qty;
                handler.read(&entId, sizeof(UInt16));
                handler.read(&itemId, sizeof(UInt16));
                handler.read(&slotId, sizeof(UInt16));
                handler.read(&qty, sizeof(UInt16));
                auto it = world.objects.find(entId);
                if(it == world.objects.end()){
                    nite::print("[client] fail SV_ADD_ITEM: entity id doesn't exist");
                    break;
                }      
                auto item = getItem(itemId, qty);
                if(item.get() == NULL){
                    nite::print("[client] fail SV_ADD_ITEM: item id "+nite::toStr(itemId)+" doesn't exist");
                    break;
                }
                auto ent = static_cast<Game::EntityBase*>(it->second.get());
                ent->invStat.add(item, slotId);                         
            } break; 
            /*
                SV_REMOVE_ITEM
            */ 
            case Game::PacketType::SV_REMOVE_ITEM: {
                if(!isSv){ break; }  
                sendAck(this->sv, handler.getOrder(), ++sentOrder);
                UInt16 entId, itemId, slotId, qty;
                handler.read(&entId, sizeof(UInt16));
                handler.read(&itemId, sizeof(UInt16));
                handler.read(&slotId, sizeof(UInt16));
                handler.read(&qty, sizeof(UInt16));
                auto it = world.objects.find(entId);
                if(it == world.objects.end()){
                    nite::print("[client] fail SV_REMOVE_ITEM: entity id doesn't exist");
                    break;
                }      
                auto item = getItem(itemId, qty);
                if(item.get() == NULL){
                    nite::print("[client] fail SV_REMOVE_ITEM: item id "+nite::toStr(itemId)+" doesn't exist");
                    break;
                }
                auto ent = static_cast<Game::EntityBase*>(it->second.get());
                ent->invStat.removeBySlotId(slotId, qty);                         
            } break;                                                                                             
            /* 
                UNKNOWN
            */
            default: {
                if(!isSv){ break; }  
                nite::print("[client] unknown packet type '"+nite::toStr(handler.getHeader())+"'");
            } break;
        


        }
    }
    // timeout
    if(nite::getTicks()-lastPacketTimeout > ClientTimeout && this->connected){
        nite::print("[client] disconnected from "+sv.str()+": timeout");
        clear();
        return;
    }
    // // ping
    if(nite::getTicks()-lastPing > 500){
        lastPing = nite::getTicks();
        nite::Packet ping(++sentOrder);
        ping.setHeader(Game::PacketType::SV_PING);
        sock.send(this->sv, ping);
    }
    // timeout connecting
    if(!this->connected && state == Game::NetState::Connecting && nite::getTicks()-lastState > Game::ConnectingTimeout){
        nite::print("[client] failed to connect to "+sv.str()+": timeout");
        clear();
        return;
    }
    updateDeliveries();
    game();
}

void Game::Client::onStart(){
    hud.start(this);
}

void Game::Client::game(){
    input.update();
    hud.update();

    // local input
    for(auto key : this->input.mapping){
        auto gk = key.second;
        auto nk = key.first;
        bool pressed = nite::keyboardPressed(nk);
        if(!pressed || entityId == 0) continue;
        auto it = world.objects.find(entityId);
        if(it == world.objects.end()) continue;
        auto ent = static_cast<Game::EntityBase*>(it->second.get());
        Game::Actionable *act = NULL;
        switch(gk){
            case Game::Key::Z: {
                act = &ent->actionables[0];
            }
            case Game::Key::X: {
                act = &ent->actionables[1];
            }
            case Game::Key::C: {
                act = &ent->actionables[2];
            }
            case Game::Key::A: {
                act = &ent->actionables[3];
            }   
            case Game::Key::S: {
                act = &ent->actionables[4];
            }                           
        }
        if(act == NULL){
            break;
        }
        switch(act->type){
            case ActionableType::None: {
                nite::print("Used none");
            } break;
            case ActionableType::Item: {

            } break;
            case ActionableType::Skill: {

            } break;            
        }
    } 

    auto buffer = input.getBuffer();
    if(buffer.size() > 0){
        nite::Packet pack(++sentOrder);
        pack.setHeader(Game::PacketType::SV_CLIENT_INPUT);        
        UInt8 amnt = buffer.size();
        pack.write(&amnt, sizeof(UInt8));
        for(int i = 0; i < buffer.size(); ++i){
            pack.write(&buffer[i].key, sizeof(UInt8));
            pack.write(&buffer[i].type, sizeof(UInt8));
            pack.write(&buffer[i].lastStroke, sizeof(UInt8));
        }
        sock.send(this->sv, pack);
    } 
    // TODO: update objs anim
    world.update();
}

void Game::Client::render(){
    nite::setColor(1.0f, 1.0f, 1.0f, 1.0f);
    nite::setRenderTarget(nite::RenderTargetGame);
    nite::setDepth(nite::DepthMiddle);
    for(auto &obj : world.objects){
        obj.second->draw();
    }
}

