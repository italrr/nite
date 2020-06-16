#include "../Engine/Input.hpp"
#include "../Engine/View.hpp"
#include "../Engine/Tools/Tools.hpp"
#include "../Engine/Graphics.hpp"
#include "../Engine/Console.hpp"
#include "Client.hpp"
#include "../Entity/Base.hpp"
#include "../Game.hpp"
#include "../UI/IngameMenu.hpp"

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
    if(!cl.connected){
        return nite::Console::Result("you must be connected a server", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
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
    WHOAMI
*/
static nite::Console::Result cfWhoami(Vector<String> params){
    auto core = Game::getGameCoreInstance();
    auto &cl = core->client;    
    
    String whoami = cl.nickname + " | " + nite::toStr(cl.clientId) + " | " + cl.sv.str() + " | " + nite::toStr(cl.entityId);
    String filter = params.size() > 0 ? nite::toLower(params[0]) : "";

    if(filter == "nickname" || filter == "nick"){
        whoami = cl.nickname;
    }

    if(filter == "clientid" || filter == "id"){
        whoami = nite::toStr(cl.clientId);
    }

    if(filter == "ip" && cl.connected){
        whoami = cl.sv.str();
    }    

    if((filter == "entity" || filter =="entityid") && cl.connected){
        whoami = nite::toStr(cl.entityId);
    }          

    return nite::Console::Result(whoami, nite::Color(1.0f, 1.0f, 0.0f, 1.0f));
}
static auto cfWhoamiIns = nite::Console::CreateFunction("cl_whoami", &cfWhoami);

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
    map = Shared<nite::Map>(NULL);
    sock.close();
    ft.clear();
    clients.clear();
    world.clear();
    deliveries.clear();
    hud.stop();
    setState(Game::NetState::Disconnected);
    // clear downloaded folder
    Vector<String> cleanMaps;
    nite::fileFind("./data/map/downloaded/", nite::Find::File, ".json", false, true, cleanMaps);
    auto indexer = nite::getIndexer();
    for(int i = 0; i < cleanMaps.size(); ++i){
        nite::removeFile(cleanMaps[i]);
        indexer->removeByPath(cleanMaps[i]);
    }         
}

void Game::Client::connect(const String &ip, UInt16 port){
    if(connected){
        nite::print("[client] failed to connect: disconnect from current server");
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
    ft.listen(nite::NetworkDefaultFileTransferPort + nite::randomInt(1, 200));
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
    this->icons.load(nite::Texture("data/texture/icons/test_icons.png"), nite::Vec2(32.0f));
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
        UInt64 netId = sender.address + sender.port + sock.sock;
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
                UInt64 svTime;
                handler.read(&svTime, sizeof(svTime));
                clock.set(svTime + ping);
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
                sendAck(this->sv, handler.getOrder(), ++sentOrder);         
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
                sendAck(this->sv, handler.getOrder(), ++sentOrder);
                UInt16 id;
                UInt16 sigId;
                float x, y;
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
                obj->net = this;
                obj->readInitialState(handler);
                world.objects[id] = obj;
                if(obj->objType == ObjectType::Entity){
                    static_cast<Game::EntityBase*>(obj.get())->loadAnim();
                }
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
            //
            /*
                SV_UPDATE_ENTITY_STANCE_STATE
            */
            case Game::PacketType::SV_UPDATE_ENTITY_STANCE_STATE: {
                // TODO: check for isLast
                if(!isSv){ break; }  
                UInt16 entId;
                UInt8 faceDirection;
                handler.read(&entId, sizeof(entId));
                handler.read(&faceDirection, sizeof(faceDirection));
                auto it = world.objects.find(entId);
                if(it != world.objects.end()){
                    auto obj = it->second;
                    if(obj->objType != ObjectType::Entity){
                        break;
                    }
                    auto ent = static_cast<Game::EntityBase*>(obj.get());
                    ent->faceDirection = faceDirection;
                    UInt8 nstate, n;
                    handler.read(&nstate, sizeof(nstate));
                    handler.read(&n, sizeof(n)); 
                    ent->setState(nstate, EntityStateSlot::BOTTOM, n);
                    handler.read(&nstate, sizeof(nstate));
                    handler.read(&n, sizeof(n)); 
                    ent->setState(nstate, EntityStateSlot::MID, n);
                }
            } break;              
            /*
                SV_UPDATE_PHYSICS_OBJECT
            */
            case Game::PacketType::SV_UPDATE_PHYSICS_OBJECT: {
                // TODO: check for isLast
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
                        auto obj = it->second;
                        obj->lerpPosition.set(x, y);
                        if(nite::abs(obj->position.x - x) > ClientRepositionThreshold.x * 2){
                            obj->setPosition(x, obj->position.y);
                        }
                        if(nite::abs(obj->position.y - y) > ClientRepositionThreshold.y * 2){
                            obj->setPosition(obj->position.x, y);
                        }                        
                        // obj->speed.set(sx, sy);
                        obj->lerpSpeed.set(sx, sy);
                    }
                }
            } break;  
            /*
                SV_UPDATE_WORLD_SIMULATION_PROPS (properties)
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
                this->camera.follow(this->entityId);
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
                auto ent = getEntity(entId);
                if(ent == NULL){
                    nite::print("[client] fail SV_SET_ENTITY_SKILLS: entity id '"+nite::toStr(entId)+"' doesn't exist");
                    break;
                }
                for(int i = 0; i < amnt; ++i){
                    UInt16 skId;
                    UInt8 lv;
                    handler.read(&skId, sizeof(UInt16));
                    handler.read(&lv, sizeof(UInt8));
                    auto sk = getSkill(skId, lv);
                    if(sk.get() == NULL){
                        nite::print("[client] warn SV_SET_ENTITY_SKILLS: skill id "+nite::toStr(skId)+" doesn't exist");
                        continue;
                    }
                    ent->skillStat.add(sk);
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
                auto ent = getEntity(entId);
                if(ent == NULL){
                    nite::print("[client] fail SV_ADD_ENTITY_SKILL: entity id '"+nite::toStr(entId)+"' doesn't exist");
                    break;
                }      
                auto sk = getSkill(skId, lv);
                if(sk.get() == NULL){
                    nite::print("[client] warn SV_ADD_ENTITY_SKILL: skill id "+nite::toStr(skId)+" doesn't exist");
                    break;
                }
                ent->skillStat.add(sk);                          
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
                auto ent = getEntity(entId);
                if(ent == NULL){
                    nite::print("[client] fail SV_REMOVE_ENTITY_SKILLS: entity id '"+nite::toStr(entId)+"' doesn't exist");
                    break;
                }
                for(int i = 0; i < amnt; ++i){
                    UInt16 skId;
                    handler.read(&skId, sizeof(UInt16));
                    if(ent->skillStat.get(skId) == NULL){
                        nite::print("[client] fail SV_REMOVE_ENTITY_SKILLS: skill id "+nite::toStr(skId)+" doesn't exist");
                        continue;
                    }
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
                auto ent = getEntity(entId);
                if(ent == NULL){
                    nite::print("[client] fail SV_REMOVE_ENTITY_SKILL: entity id '"+nite::toStr(entId)+"' doesn't exist");
                    break;
                }      
                if(ent->skillStat.get(skId) == NULL){
                    nite::print("[client] fail SV_REMOVE_ENTITY_SKILL: skill id "+nite::toStr(skId)+" doesn't exist");
                    break;
                }
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
                auto ent = getEntity(entId);
                if(ent == NULL){
                    nite::print("[client] fail SV_SET_ENTITY_ACTIONABLES: entity id '"+nite::toStr(entId)+"' doesn't exist");
                    break;
                }
                for(int i = 0; i < amnt; ++i){
                    UInt8 slot;
                    handler.read(&slot, sizeof(slot));
                    if(slot > Game::EntityActionables){
                        slot = 0;
                        nite::print("[client] fail SV_SET_ENTITY_ACTIONABLES: offset slot");
                    }
                    handler.read(&ent->actionables[slot].type, sizeof(UInt8));
                    handler.read(&ent->actionables[slot].id, sizeof(UInt32));
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
                eff->readInitialState(handler);                         
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
                SV_UPDATE_ENTITY_ALL_STAT
            */ 
            case Game::PacketType::SV_UPDATE_ENTITY_ALL_STAT: {
                if(!isSv){ break; }  
                sendAck(this->sv, handler.getOrder(), ++sentOrder);
                UInt16 entId;
                handler.read(&entId, sizeof(UInt16));
                auto it = world.objects.find(entId);
                if(it == world.objects.end()){
                    nite::print("[client] fail SV_UPDATE_ENTITY_ALL_STAT: entity id doesn't exist");
                    break;
                }      
                auto ent = static_cast<Game::EntityBase*>(it->second.get()); // its an entity
                ent->readAllStatState(handler);
            } break; 
            /*
                SV_UPDATE_ENTITY_HEALTH_STAT
            */ 
            case Game::PacketType::SV_UPDATE_ENTITY_HEALTH_STAT: {
                if(!isSv){ break; }  
                sendAck(this->sv, handler.getOrder(), ++sentOrder);
                UInt16 entId;
                handler.read(&entId, sizeof(UInt16));
                auto it = world.objects.find(entId);
                if(it == world.objects.end()){
                    nite::print("[client] fail SV_UPDATE_ENTITY_HEALTH_STAT: entity id doesn't exist");
                    break;
                }      
                auto ent = static_cast<Game::EntityBase*>(it->second.get()); // its an entity
                ent->readHealthStatState(handler);
            } break; 
            /*
                SV_NOTIFY_ENTITY_DEATH
            */ 
            case Game::PacketType::SV_NOTIFY_ENTITY_DEATH: {
                if(!isSv){ break; }  
                sendAck(this->sv, handler.getOrder(), ++sentOrder);
                UInt16 entId;
                handler.read(&entId, sizeof(UInt16));
                auto it = world.objects.find(entId);
                if(it == world.objects.end()){
                    nite::print("[client] fail SV_NOTIFY_ENTITY_DEATH: entity id doesn't exist");
                    break;
                }      
                auto ent = static_cast<Game::EntityBase*>(it->second.get());
                if(ent->healthStat.dead){
                    break; // already dead bro
                }
                ent->readHealthStatState(handler);
                ent->onDeath();
                if(ent->id == entityId){
                    nite::print("your character dead");
                }
            } break;  
            /*
                SV_NOTIFY_ENTITY_REVIVE
            */ 
            case Game::PacketType::SV_NOTIFY_ENTITY_REVIVE: {
                if(!isSv){ break; }  
                sendAck(this->sv, handler.getOrder(), ++sentOrder);
                UInt16 entId;
                handler.read(&entId, sizeof(UInt16));
                auto it = world.objects.find(entId);
                if(it == world.objects.end()){
                    nite::print("[client] fail SV_NOTIFY_ENTITY_REVIVE: entity id doesn't exist");
                    break;
                }     
                // **TODO** 
                break;
            } break; 
            /*
                SV_NOTIFY_GAME_OVER
            */ 
            case Game::PacketType::SV_SET_GAME_OVER: {
                if(!isSv){ break; }  
                sendAck(this->sv, handler.getOrder(), ++sentOrder);
                nite::print("game over!");
                // **TODO** 
            } break;                                      
            /*
                SV_NOTIFY_GAME_RESTART
            */ 
            case Game::PacketType::SV_SET_GAME_RESTART: {
                if(!isSv){ break; }  
                sendAck(this->sv, handler.getOrder(), ++sentOrder);
                nite::print("restarting...");
                world.objects.clear(); // manually clear world (TODO: might be dangerous?)
                // **TODO** 
            } break;   
            /*
                SV_UPDATE_SKILL_STATE
            */ 
            case Game::PacketType::SV_UPDATE_SKILL_STATE: {
                if(!isSv){ break; }  
                sendAck(this->sv, handler.getOrder(), ++sentOrder);
                UInt16 entId, skId;
                handler.read(&entId, sizeof(entId));
                handler.read(&skId, sizeof(skId));
                auto ent = getEntity(entId);
                if(ent == NULL){
                    nite::print("[client] server sent skill update for unexisting entity id '"+nite::toStr(entId)+"'");
                    break;
                }
                auto sk = ent->skillStat.get(skId);
                if(sk == NULL){
                    nite::print("[client] fail to update skill id '"+nite::toStr(skId)+"' from entity id '"+nite::toStr(entId)+"': it doesn't posses this skill");
                    break;
                }
                sk->readUpdate(handler);
            } break;               
            /*
                SV_AWAIT_CLIENT_LOAD
            */ 
            case Game::PacketType::SV_AWAIT_CLIENT_LOAD: {
                if(!isSv){ break; }  
                sendAck(this->sv, handler.getOrder(), ++sentOrder);
                String hash;
                handler.read(hash);
                auto indexer = nite::getIndexer();
                auto ifile = indexer->get(hash);
                // TODO: handle other resources like tilesources and the actual bitmap
                if(ifile == NULL){
                    auto me = this;
                    auto who = nite::IP_Port(this->sv.ip, nite::NetworkDefaultFileTransferPort);
                    String output = "./data/map/downloaded/"+hash+".json";
                    nite::print("downloading map from server...");
                    ft.request(who, hash, output, [me](const nite::IndexedFile &file, bool success){
                        if(!success){
                            nite::print("map file is corrupted");
                            nite::removeFile(file.path);
                            auto indexer = nite::getIndexer();
                            indexer->removeByPath(file.path);
                            // TODO: retry when it arrives corrupted
                            nite::AsyncTask::spawn([me](nite::AsyncTask::Context &ctx){
                                nite::Packet ready(++me->sentOrder);
                                ready.setHeader(Game::PacketType::SV_CLIENT_LOAD_READY);
                                me->persSend(me->sv, ready, 1000, -1);
                                ctx.stop();
                            }, 100);
                            return;
                        }
                        nite::print("[client] downloaded map '"+file.path+"'. about to load it");
                        nite::SmallPacket payload;
                        payload.write(&file, sizeof(file));
                        nite::AsyncTask::spawn([me](nite::AsyncTask::Context &ctx){
                            nite::IndexedFile file;
                            ctx.payload.read(&file, sizeof(file));
                            nite::Packet ready(++me->sentOrder);
                            ready.setHeader(Game::PacketType::SV_CLIENT_LOAD_READY);
                            me->persSend(me->sv, ready, 1000, -1);
                            auto map = Shared<nite::Map>(new nite::Map());
                            map->load(file.path);
                            me->map = map;
                            ctx.stop();
                        }, 100, payload);
                    });
                }else{
                    auto map = Shared<nite::Map>(new nite::Map());
                    map->load(ifile->path);
                    this->map = map;
                    nite::Packet ready(++this->sentOrder);
                    ready.setHeader(Game::PacketType::SV_CLIENT_LOAD_READY);
                    this->persSend(this->sv, ready, 1000, -1);
                }
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
    // ping
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
    igmenu.start(this);
    camera.start(this);
}

void Game::Client::game(){
    input.update(igmenu.open);
    hud.update();
    igmenu.update();
    // local input
    for(auto key : this->input.mapping){
        auto gk = key.second;
        auto nk = key.first;
        bool pressed = (nk > 200 ? nite::mousePressed(nk) : nite::keyboardPressed(nk));
        if(!pressed || entityId == 0) continue;
        auto ent = getEntity(this->entityId);
        if(ent == NULL){
            continue;
        }
        Game::Actionable *act = NULL;
        auto requestUse = [&](UInt8 type, UInt32 id, UInt16 target, float x, float y){
            nite::Packet use(++this->sentOrder);
            use.setHeader(Game::PacketType::SV_ENTITY_USE_SKILL_ITEM);
            use.write(&this->entityId, sizeof(this->entityId));
            use.write(&type, sizeof(type));
            use.write(&id, sizeof(id));
            use.write(&target, sizeof(target));
            use.write(&x, sizeof(x));
            use.write(&y, sizeof(y));
            this->persSend(this->sv, use, 1000, -1);
        };
        switch(gk){
            case Game::Key::k1: {
                act = &ent->actionables[0];
            } break;
            case Game::Key::k2: {
                act = &ent->actionables[1];
            } break;
            case Game::Key::k3: {
                act = &ent->actionables[2];
            } break;
            case Game::Key::k4: {
                act = &ent->actionables[3];
            } break;
            case Game::Key::k5: {
                act = &ent->actionables[4];
            } break;
            case Game::Key::k6: {
                act = &ent->actionables[5];
            } break;    
            case Game::Key::k7: {
                act = &ent->actionables[6];
            } break;                                        
        }
        if(act == NULL){
            break;
        }
        switch(act->type){
            case ActionableType::None: {
                nite::print("Used none");
            } break;
            case ActionableType::Item: {
                nite::print("Used Item");
            } break;
            case ActionableType::Skill: {
                float x = ent->position.x, y = ent->position.y;
                auto sk = ent->skillStat.get(act->id);
                if(sk != NULL && sk->usageType == SkillUsageType::Target){
                    x = nite::mouseX() + nite::getViewX(nite::RenderTargetGame);
                    y = nite::mouseY() + nite::getViewY(nite::RenderTargetGame);
                }
                // notify server this player wants to use an item/skill
                requestUse(ActionableType::Skill, act->id, 0, x, y);
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
    // client-side interpolation for new-position smoothing-out (lag compensation)
    // might make client to be a few milliseconds behind, which is concerning
    // we'll have to test this out in the future
    for(auto &obj : world.objects){
        obj.second->position.lerpDiscrete(obj.second->lerpPosition, 0.15f);
        obj.second->speed.lerpDiscrete(obj.second->lerpSpeed, 0.55f);
    }    
    // TODO: update objs anim
    world.update();
    camera.update();   
}

Game::EntityBase *Game::Client::getEntity(UInt16 id){
    if(id == 0){
        return NULL;
    }    
    auto obj = this->world.get(id);
    if(obj.get() == NULL){
        return NULL;
    }
    if(obj->objType != Game::ObjectType::Entity){
        return NULL;
    }
    return static_cast<Game::EntityBase*>(obj.get());
}

void Game::Client::render(){
    nite::setColor(1.0f, 1.0f, 1.0f, 1.0f);
    nite::setRenderTarget(nite::RenderTargetGame);
    nite::setDepth(nite::DepthMiddle);
    if(map != NULL){
        map->render(0, 0, 0, 0); // TODO implement viewport using camera
    }
    for(auto &obj : world.objects){
        obj.second->draw();
    }
}

