#include "../Engine/Input.hpp"
#include "../Engine/View.hpp"
#include "../Engine/Tools/Tools.hpp"
#include "../Engine/Graphics.hpp"
#include "../Engine/Console.hpp"
#include "../Engine/Map.hpp"
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
    nite::Packet cmd(Game::PacketType::SV_REMOTE_CMD_EXEC);
    
    cmd.write(input);
    cl.sendPersPacketFor(cl.sv, cmd, ++cl.svAck);
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
    nite::Packet rcon(Game::PacketType::SV_RCON);
    rcon.write(hash);
    cl.sendPersPacketFor(cl.sv, rcon, ++cl.svAck);

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
    nite::Console::CreateProxy clPhysicsDebug("cl_physics_debug", nite::Console::ProxyType::Bool, sizeof(bool), &world.debug);
    // nite::Console::CreateProxy clTimescale("cl_local_timescale", nite::Console::ProxyType::Float, sizeof(float), &world.timescale);
    init = false;
    isServer = false;
    clear();
    traps.start(this);
}

Game::Client::~Client(){

}

void Game::Client::clear(){
    svAck = 0;
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
    nite::Texture icons = nite::Texture("data/texture/icons/test_icons.png");
    this->icons.load(icons, nite::Vec2(32.0f));
    if(!debugFont.isLoaded()){
        debugFont.load("data/font/SpaceMono-Regular.ttf", 16, 3.0f);
    }
}

void Game::Client::disconnect(){
    disconnect("");
}

void Game::Client::disconnect(const String &reason){
    if(!init || state == Game::NetState::Disconnected){
        return;
    }
    nite::Packet disc;
    disc.setOrder(++sentOrder);
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
    nite::Packet pack(Game::PacketType::SV_CHAT_MESSAGE);
    pack.write(&clientId, sizeof(UInt64));
    pack.write(msg);
    sendPersPacketFor(sv, pack, ++svAck);
}

static int bytes = 0;
static int many = 0;
static UInt64 lastcheck = nite::getTicks();
static String lastChange = "";
void Game::Client::update(){
    // exit if no connection
    if(state == Game::NetState::Disconnected || !init){
        return;
    }

    if(removeQueue.size() > 0){
        for(int i = 0; i < removeQueue.size(); ++i){
            auto *obj = world.get(removeQueue[i]);
            if(obj == NULL){
                continue;
            }
            world.objects.erase(obj->id);
            removeQueue.erase(removeQueue.begin() + i);
            --i;
        }
    }

    pointer = nite::getView(nite::RenderTargetGame) + nite::mousePosition();

    nite::Packet handler;
    nite::IP_Port sender;

    int nb = sock.recv(sender, handler);
    if(nb > 0){
        bytes += nb;
        many += 1;
        UInt64 netId = sender.address + sender.port + sock.sock;
        bool isSv = netId == this->serverId && state != Game::NetState::Disconnected;
        bool isLast = isSv && handler.getOrder() > rcvOrder;
        if(isSv && isLast){
            lastPacket = handler;
            rcvOrder = handler.getOrder();
        }
        switch(handler.getHeader()){
            /*
                SV_PONG
            */
            case Game::PacketType::SV_PONG: {
                if(!isSv || !isLast){ break; }
                lastPacketTimeout = nite::getTicks();
                this->ping = nite::getTicks() - this->lastPing;            
            } break;
            /*
                SV_PING
            */
            case Game::PacketType::SV_PING: {
                if(!isSv || !isLast){ break; }
                nite::Packet pong(Game::PacketType::SV_PONG);
                pong.setOrder(++sentOrder);
                sock.send(this->sv, pong);
            } break;
            /*
                SV_CONNECT_REJECT
            */
            case Game::PacketType::SV_CONNECT_REJECT: {
                if(!isSv){ break; }
                if(state != Game::NetState::Connecting){
                    break;
                }
                String reason;
                handler.read(reason);
                nite::print("[client] connection rejected: "+reason);
                clear();
            } break;
            /*
                SV_CLIENT_DROP
            */
            case Game::PacketType::SV_CLIENT_DROP: {
                if(!isSv || !isLast){ break; }
                String reason;
                handler.read(reason);
                if(reason.length() == 0){
                    reason = "no reason";
                }
                nite::print("[client] server dropped connection: "+reason);
                clear();
            } break;               
            default: {
                rcvPackets.push_back(handler);
            } break;                              
        }
    }


    if(nite::getTicks()-lastcheck > 1000){
        lastChange = "packets "+nite::toStr(many)+"/s | bytes "+nite::toStr(bytes)+"/s";
        // nite::print("ping "+nite::toStr(this->ping)+"ms | elapsed "+nite::toStr(nite::getTicks()-lastcheck)+" | total "+nite::toStr(many)+" | sent "+nite::toStr(bytes));
        bytes = 0;
        many = 0;
        lastcheck = nite::getTicks();
    }

    // timeout
    if(nite::getTicks()-lastPacketTimeout > ClientTimeout && this->connected){
        nite::print("[client] disconnected from "+sv.str()+": timeout");
        clear();
        return;
    }
    // ping
    if(nite::getTicks()-lastSentPing > 500){
        lastSentPing = nite::getTicks();
        lastPing = nite::getTicks();
        nite::Packet ping(Game::PacketType::SV_PING);
        ping.setOrder(++sentOrder);
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
    nite::Console::interpret("cl_camera_forcezoom true", false, false, false, false);
    hud.start(this);
    igmenu.start(this);
    camera.start(this);
}

void __temp();

void Game::Client::processIncomPackets(){
    std::function<void(nite::Packet &handler, bool ignoreOrder)> processPacket = [&](nite::Packet &handler, bool ignoreOrder){
        auto sender = handler.sender;
        UInt64 netId = sender.address + sender.port + sock.sock;
        bool isSv = this->sv.isSame(handler.sender) && state != Game::NetState::Disconnected;
        bool isLast = isSv && handler.getOrder() > rcvOrder;
        if(isSv && isLast && !ignoreOrder){
            lastPacket = handler;
            lastPacketTimeout = nite::getTicks();
            rcvOrder = handler.getOrder();
        }           
        // for multi-part packets
        if(ignoreOrder){
            isLast = true;
        }
        switch(handler.getHeader()){
            /*
                SV_MULTI_PART_PACKET    
            */
            case Game::PacketType::SV_MULTI_PART_PACKET: {
                Vector<UInt16> sizes;
                UInt8 total;
                handler.read(&total, sizeof(total));
                for(int i = 0; i < total; ++i){
                    UInt16 size;
                    handler.read(&size, sizeof(size));
                    sizes.push_back(size);
                }
                for(int i = 0; i < total; ++i){
                    nite::Packet holder;
                    handler.read(holder.data, sizes[i]);
                    processPacket(holder, true);
                }
            } break;    
            /*
                SV_CONNECT_ACCEPT
            */
            case Game::PacketType::SV_CONNECT_ACCEPT: {
                sendAck(sender, handler.getAck());
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
                    nite::print("[client] connected to "+this->sv.str()+" | your clientId is "+nite::toStr(this->clientId));
                }
            } break;                               
            /*
                SV_ACK
            */
            case Game::PacketType::SV_ACK: {
                ack(handler);
            } break;
            /*
                SV_CLIENT_JOIN
            */
            case Game::PacketType::SV_CLIENT_JOIN: {
                if(!isSv || !isLast){ break; }
                UInt64 uid;
                String nick;
                handler.read(&uid, sizeof(UInt64));
                handler.read(nick);
                this->clients[uid] = Game::ClClient(uid, nick);
                nite::print(nick+" joined the game");
                // TODO: add in-game notification for this message
                sendAck(sender, handler.getAck());
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
                sendAck(sender, handler.getAck());
            } break;
            /*
                SV_CLIENT_LIST
            */
            case Game::PacketType::SV_CLIENT_LIST: {
                if(!isSv || !isLast){ break; }
                sendAck(this->sv, handler.getAck());
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
                if(!isSv || !isLast){ break; }
                String msg;
                handler.read(msg);
                nite::print(msg);
                // TODO: add in-game notification for this message (and chat)
                sendAck(this->sv, handler.getAck());
            } break;
            /*
                SV_CHAT_MESSAGE
            */
            case Game::PacketType::SV_CHAT_MESSAGE: {
                    if(!isSv || !isLast){ break; }
                    UInt64 uid;
                    String msg;
                    handler.read(&uid, sizeof(UInt64));
                    handler.read(msg);
                    auto it = clients.find(uid);
                    String who = it != clients.end() ? it->second.nickname : "???";
                    nite::print(this->nickname+" ["+who+"] "+msg);
                    // TODO: add in-game chat
                    sendAck(this->sv, handler.getAck());
            } break;
            /*
                SV_CREATE_OBJECT
            */
            case Game::PacketType::SV_CREATE_OBJECT: {
                if(!isSv || !isLast){ break; }
                sendAck(this->sv, handler.getAck());
                UInt16 id;
                UInt16 sigId;
                float x, y;
                handler.read(&id, sizeof(UInt16));
                handler.read(&sigId, sizeof(Int16));
                handler.read(&x, sizeof(float));
                handler.read(&y, sizeof(float));
                auto obj = createNetObject(sigId, x, y);
                if(obj.get() == NULL){
                    nite::print("[client] fail SV_CREATE_OBJECT: undefined obj sig '"+Game::ObjectSig::name(sigId)+"' on the client");
                    break;
                }
                if(world.objects.find(id) != world.objects.end()){
                    nite::print("[client] fail SV_CREATE_OBJECT: duplicated id "+nite::toStr(id)+"");
                    // TODO: come up with a way to properly handle duplicated ids?
                    break;
                }
                obj->net = this;
                obj->readInitialState(handler);
                obj->id = id;
                world.add(obj, obj->position.x, obj->position.y);
                if(obj->objType == ObjectType::Entity){
                    static_cast<Game::EntityBase*>(obj.get())->loadAnim();
                }
                nite::print("[client] spawned object: '"+Game::ObjectSig::name(sigId)+"' id: "+nite::toStr(id)+", type: '"+Game::ObjectType::name(obj->objType)+"', sigId: "+nite::toStr(sigId)+" at "+obj->position.str());
            } break;
            /*
                SV_DESTROY_OBJECT
            */
            case Game::PacketType::SV_DESTROY_OBJECT: {
                if(!isSv || !isLast){ break; }
                sendAck(this->sv, handler.getAck());
                UInt16 id;
                handler.read(&id, sizeof(UInt16));
                auto obj = world.objects.find(id);
                if(obj == world.objects.end()){
                    nite::print("[client] fail SV_DESTROY_OBJECT: object id "+nite::toStr(id)+" doesn't exist");
                    break;
                }
                nite::print("received destroy obj id "+nite::toStr(obj->second->id));
                removeQueue.push_back(obj->second->id);
            } break;
            /*
                SV_UPDATE_ENTITY_STANCE_STATE
            */
            case Game::PacketType::SV_UPDATE_ENTITY_STANCE_STATE: {
                if(!isSv || !isLast){ break; }
                UInt16 entId;
                UInt8 faceDirection;
                handler.read(&entId, sizeof(entId));
                handler.read(&faceDirection, sizeof(faceDirection));
                auto ent = getEntity(entId);
                if(ent != NULL){
                    ent->faceDirection = faceDirection;
                    UInt8 nstate, n;
                    for(int i = 0; i < EntityStateSlot::total; ++i){
                        if(i == EntityStateSlot::SPECIAL){ //specials are passed directly since they don't involve animations                     
                            handler.read(&ent->state[i], sizeof(ent->state[i]));
                            handler.read(&ent->stNum[i], sizeof(ent->stNum[i]));
                            ent->lastStateTime[i] = nite::getTicks();
                            ent->lastFrameTime[i] = nite::getTicks();
                        }else{
                            handler.read(&nstate, sizeof(nstate));
                            handler.read(&n, sizeof(n));
                            ent->setState(nstate, i, n);
                        }
                        handler.read(&ent->lastExpectedTime[i], sizeof(ent->lastExpectedTime[i])); // pass expected time directly
                    }
                }
            } break;
            /*
                SV_UPDATE_WORLD_SIMULATION_PROPS (properties)
            */
            case Game::PacketType::SV_UPDATE_WORLD_SIMULATION_PROPS: {
                if(!isSv || !isLast){ break; }
                sendAck(this->sv, handler.getAck());                    
                handler.read(&gameTimescale, sizeof(gameTimescale));
                handler.read(&gameTickRate, sizeof(gameTickRate));
                // this->world.tickrate = gameTickRate;
                // this->world.timescale = gameTimescale;
            } break;
            /*
                SV_UPDATE_OBJECT_RELATIVE_TIMESCALE
            */
            case Game::PacketType::SV_UPDATE_OBJECT_RELATIVE_TIMESCALE: {
                if(!isSv || !isLast){ break; }
                sendAck(this->sv, handler.getAck());
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
                if(!isSv || !isLast){ break; }
                sendAck(this->sv, handler.getAck());
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
                if(!isSv || !isLast){ break; }
                sendAck(this->sv, handler.getAck());
                this->onStart();
            } break;
            /*
                SV_NOTI_ENTITY_OWNER
            */
            case Game::PacketType::SV_NOTI_ENTITY_OWNER: {
                if(!isSv || !isLast){ break; }
                sendAck(this->sv, handler.getAck());
                handler.read(&this->entityId, sizeof(UInt16));
                this->hud.follow(this->entityId);
                this->camera.follow(this->entityId);
            } break;

            /*
                SV_SET_ENTITY_SKILLS
            */
            case Game::PacketType::SV_SET_ENTITY_SKILLS: {
                if(!isSv || !isLast){ break; }
                sendAck(this->sv, handler.getAck());
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
                if(!isSv || !isLast){ break; }
                sendAck(this->sv, handler.getAck());
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
                if(!isSv || !isLast){ break; }
                sendAck(this->sv, handler.getAck());
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
                if(!isSv || !isLast){ break; }
                sendAck(this->sv, handler.getAck());
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
                if(!isSv || !isLast){ break; }
                sendAck(this->sv, handler.getAck());
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
                if(!isSv || !isLast){ break; }
                sendAck(this->sv, handler.getAck());
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
                if(!isSv || !isLast){ break; }
                sendAck(this->sv, handler.getAck());
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
                if(!isSv || !isLast){ break; }
                sendAck(this->sv, handler.getAck());
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
                if(!isSv || !isLast){ break; }
                sendAck(this->sv, handler.getAck());
                UInt16 entId, itemId, slotId, qty;
                UInt8 upgLv;
                UInt16 compound[GAME_ITEM_MAX_COMPOUND_SLOTS];
                handler.read(&entId, sizeof(UInt16));
                handler.read(&itemId, sizeof(UInt16));
                handler.read(&slotId, sizeof(UInt16));
                handler.read(&qty, sizeof(UInt16));
                handler.read(&upgLv, sizeof(upgLv));
                for(int i = 0; i < GAME_ITEM_MAX_COMPOUND_SLOTS; ++i){
                    handler.read(&compound[i], sizeof(compound[i]));
                }
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
                for(int i = 0; i < GAME_ITEM_MAX_COMPOUND_SLOTS; ++i){
                    item->compound[i] = compound[i];
                }
                item->upgradelv = upgLv;
                auto ent = static_cast<Game::EntityBase*>(it->second.get());
                ent->invStat.add(item, slotId);
            } break;
            /*
                SV_REMOVE_ITEM
            */
            case Game::PacketType::SV_REMOVE_ITEM: {
                if(!isSv || !isLast){ break; }
                sendAck(this->sv, handler.getAck());
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
                SV_UPDATE_ENTITY_INVENTORY_SLOTS
            */
            case Game::PacketType::SV_UPDATE_ENTITY_INVENTORY_SLOTS: {
                if(!isSv || !isLast){ break; }
                sendAck(this->sv, handler.getAck());
                UInt16 entId;
                handler.read(&entId, sizeof(UInt16));
                auto it = world.objects.find(entId);
                if(it == world.objects.end()){
                    nite::print("[client] fail SV_UPDATE_ENTITY_INVENTORY_SLOTS: entity id doesn't exist");
                    break;
                }
                auto ent = static_cast<Game::EntityBase*>(it->second.get());
                ent->readInvSlotsState(handler);
            } break;
            /*
                SV_UPDATE_ENTITY_INVENTORY_CARRY
            */
            case Game::PacketType::SV_UPDATE_ENTITY_INVENTORY_CARRY: {
                if(!isSv || !isLast){ break; }
                sendAck(this->sv, handler.getAck());
                UInt16 entId;
                handler.read(&entId, sizeof(UInt16));
                auto it = world.objects.find(entId);
                if(it == world.objects.end()){
                    nite::print("[client] fail SV_UPDATE_ENTITY_INVENTORY_CARRY: entity id doesn't exist");
                    break;
                }
                auto ent = static_cast<Game::EntityBase*>(it->second.get());
                ent->readInvListState(handler);
                ent->readInvSlotsState(handler);
            } break;
            /*
                SV_UPDATE_ENTITY_ALL_STAT
            */
            case Game::PacketType::SV_UPDATE_ENTITY_ALL_STAT: {
                if(!isSv || !isLast){ break; }
                sendAck(this->sv, handler.getAck());
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
                if(!isSv || !isLast){ break; }
                sendAck(this->sv, handler.getAck());
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
                if(!isSv || !isLast){ break; }
                sendAck(this->sv, handler.getAck());
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
                if(!isSv || !isLast){ break; }
                sendAck(this->sv, handler.getAck());
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
                if(!isSv || !isLast){ break; }
                sendAck(this->sv, handler.getAck());
                nite::print("game over!");
                // **TODO**
            } break;
            /*
                SV_NOTIFY_GAME_RESTART
            */
            case Game::PacketType::SV_SET_GAME_RESTART: {
                if(!isSv || !isLast){ break; }
                sendAck(this->sv, handler.getAck());
                nite::print("restarting...");
                world.objects.clear(); // manually clear world (TODO: might be dangerous?)
                // **TODO**
            } break;
            /*
                SV_UPDATE_SKILL_STATE
            */
            case Game::PacketType::SV_UPDATE_SKILL_STATE: {
                if(!isSv || !isLast){ break; }
                sendAck(this->sv, handler.getAck());
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
                SV_UPDATE_TRAP_STATE
            */
            case Game::PacketType::SV_UPDATE_TRAP_STATE: {
                if(!isSv || !isLast){ break; }
                UInt16 id, state;
                handler.read(&id, sizeof(id));
                handler.read(&state, sizeof(state));
                auto trap = this->traps.getTrap(id);
                if(trap == NULL){
                    nite::print("[client] SV_UPDATE_TRAP_STATE: unable to find id '"+nite::toStr(id)+"'");
                    break;
                }
                // trap->setState(state, this->map, this->world);
            } break;
            /*
                SV_UPDATE_MANY_TRAPS_STATE
            */
            case Game::PacketType::SV_UPDATE_MANY_TRAPS_STATE: {
                if(!isSv || !isLast){ break; }
                UInt16 n;
                UInt16 id;
                UInt8 state;
                handler.read(&n, sizeof(n));
                for(int i = 0; i < n; ++i){
                    handler.read(&id, sizeof(id));
                    handler.read(&state, sizeof(state));
                    auto trap = this->traps.getTrap(id);
                    if(trap == NULL){
                        nite::print("[client] SV_UPDATE_TRAP_STATE: unable to find id '"+nite::toStr(id)+"'");
                        break;
                    }
                    // trap->setState(state, this->map, this->world);
                }
            } break;
            /*
                SV_AWAIT_CLIENT_LOAD
            */
            case Game::PacketType::SV_AWAIT_CLIENT_LOAD: {
                if(!isSv || !isLast){ break; }
                sendAck(this->sv, handler.getAck());
                String hash;
                handler.read(hash);
                auto indexer = nite::getIndexer();
                indexer->indexDir("./data/map/generated/"); // in case the map is already in this client's data folder
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
                                nite::Packet ready(Game::PacketType::SV_CLIENT_LOAD_READY);
                                me->sendPersPacketFor(me->sv, ready, ++me->svAck);
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
                            nite::Packet ready(Game::PacketType::SV_CLIENT_LOAD_READY);
                            me->sendPersPacketFor(me->sv, ready, ++me->svAck);
                            auto map = Shared<nite::Map>(new nite::Map());
                            map->load(file.path);
                            me->map = map;
                            ctx.stop();
                        }, 100, payload);
                    });
                }else{
                    auto map = Shared<nite::Map>(new nite::Map());
                    map->load(ifile->path);
                    setCurrentMap(map);
                    nite::Packet ready(Game::PacketType::SV_CLIENT_LOAD_READY);
                    this->sendPersPacketFor(this->sv, ready, ++this->svAck);
                }
            } break;
            /*
                SV_UPDATE_ENTITY_SET_CASTING_STATE
            */
            case Game::PacketType::SV_UPDATE_ENTITY_SET_CASTING_STATE: {
                if(!isSv || !isLast){ break; }
                sendAck(this->sv, handler.getAck());
                UInt16 entId, target;
                UInt32 id;
                UInt8 type;
                UInt64 startTime, time;
                float x, y;
                handler.read(&entId, sizeof(entId));
                handler.read(&id, sizeof(id));
                handler.read(&type, sizeof(type));
                handler.read(&target, sizeof(target));
                handler.read(&startTime, sizeof(startTime));
                handler.read(&time, sizeof(time));
                handler.read(&x, sizeof(x));
                handler.read(&y, sizeof(y));
                auto ent = getEntity(entId);
                if(ent != NULL){
                    if(ent->currentCasting.get() == NULL){
                        ent->currentCasting = Shared<Game::EntityCasting>(new Game::EntityCasting());
                    }
                    ent->currentCasting->id = id;
                    ent->currentCasting->type = type;
                    ent->currentCasting->target = target;
                    ent->currentCasting->time = time;
                    ent->currentCasting->startTime = startTime;
                    ent->currentCasting->p.set(x,y);
                }
            } break;
            /*
                SV_NOTIFY_ENTITY_DAMAGE
            */
            case Game::PacketType::SV_NOTIFY_ENTITY_DAMAGE: {
                if(!isSv || !isLast){ break; }
                sendAck(this->sv, handler.getAck());
                UInt16 entId;
                UInt32 amnt;
                nite::Vec2 p;
                handler.read(&entId, sizeof(entId));
                handler.read(&amnt, sizeof(amnt));
                handler.read(&p.x, sizeof(p.x));
                handler.read(&p.y, sizeof(p.y));
                auto ent = getEntity(entId);
                if(ent == NULL){
                    nite::print("[client] SV_NOTIFY_ENTITY_DAMAGE: unable to find entity id '"+nite::toStr(entId)+"'");
                    break;
                }
                ent->markDamaged();
                ent->addDamageCountShow(amnt);
                if(p.x > 0 && p.y > 0){
                    auto ef = Shared<Game::VfxBang>(new Game::VfxBang());
                    ef->position = p;
                    this->vfx.add(ef);
                }
                if(entId == entityId){
                    nite::shakeScreen(nite::RenderTargetGame, 0.55f, 300 * (amnt / 5));
                }
            } break;  
            /*
                SV_SET_OBJECT_POSITION
            */
            case Game::PacketType::SV_SET_OBJECT_POSITION: {
                if(!isSv || !isLast){ break; }
                sendAck(this->sv, handler.getAck());
                UInt16 id;
                nite::Vec2 p;
                handler.read(&id, sizeof(id));
                handler.read(&p.x, sizeof(p.x));
                handler.read(&p.y, sizeof(p.y));
                auto obj = world.get(id);
                if(obj != NULL){
                    // obj->setPosition(p);
                }

            } break;                    
            /*
                UNKNOWN
            */
            default: {
                if(!isSv){ break; }
                nite::print("[client] unknown packet type '"+nite::toStr(handler.getHeader())+"'");
            } break;
            /*
                SV_SET_OBJECT_POSITION
            */            
            case Game::PacketType::SV_UPDATE_OBJECT_STATE: {
                if(!isSv || !isLast){ break; }
                UInt32 ndelta = handler.getOrder();
                UInt8 n;
                handler.read(&n, sizeof(n));
                for(int i = 0; i < n; ++i){
                    // read values
                    Game::ObjectState st;
                    handler.read(&st.objId, sizeof(st.objId));
                    handler.read(&st.states, sizeof(st.states));
                    if(hasIssuedDeltaStateUpdate(DeltaUpdateType::ANIMATION, st.states)){
                        handler.read(&st.faceDir, sizeof(st.faceDir));
                        for(int j = 0; j < EntityStateSlot::total; ++j){
                            handler.read(&st.animSt[j], sizeof(st.animSt[j]));
                            handler.read(&st.animNum[j], sizeof(st.animNum[j]));
                            handler.read(&st.animExtime[j], sizeof(st.animExtime[j]));
                        }                                   
                    }
                    if(hasIssuedDeltaStateUpdate(DeltaUpdateType::PHYSICS, st.states)){
                        UInt16 currentP;
                        UInt8 nroute;
                        UInt32 c;
                        handler.read(&currentP, sizeof(currentP));
                        handler.read(&st.total, sizeof(st.total));                             
                        handler.read(&nroute, sizeof(nroute));    
                        st.position = world.toCoors(currentP);
                        for(int j = 0; j < nroute; ++j){
                            handler.read(&c, sizeof(c));    
                            st.route.route.push_back(nite::MapCell(c, 0, world.width));
                        }
                    }   
                    // apply them     
                    auto obj = world.get(st.objId);
                    if(obj == NULL){
                        nite::print("[client] SV_UPDATE_OBJECT_STATE: unable to find obj id '"+nite::toStr(st.objId)+"'");
                        break;
                    }
                    obj->currentState = st;
                    if(hasIssuedDeltaStateUpdate(DeltaUpdateType::PHYSICS, st.states)){
                        obj->setMoveRoute(st.route, st.total);
                    }
                }
            } break; 
        }
    };

    for(int i = 0; i < rcvPackets.size(); ++i){
        processPacket(rcvPackets[i], false);
    }

    rcvPackets.clear();
}

void Game::Client::deliverPacketQueue(){
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
        sock.send(sv, handle);
    }
    packetQueue.clear();
}

void Game::Client::player(){
    input.update(false);
    igmenu.update();
    auto ent = getEntity(this->entityId);
    bool isSpace = input.isKeyPress(Game::Key::SPACE);
    // local input
    if(ent != NULL){
        // ent->pointingAt = pointer;
        if(nite::keyboardPressed(nite::keyNUMPAD0)){
            String inv = "inv ";
            String wear = "wear ";
            for(auto &it : ent->invStat.carry){
                inv += it.second->name+"(x"+nite::toStr(it.second->qty)+"), ";
            }
            for(int i = 0; i < EquipSlot::TOTAL; ++i){
                if(ent->invStat.slots[i] == NULL){
                    continue;
                }
                wear += ent->invStat.slots[i]->name+"("+EquipSlot::name(i)+"), ";
            }
            nite::print(inv+"\n"+wear);
        }
    }
    if(nite::mousePressed(nite::butLEFT)){
        UInt32 i = world.toIndex(nite::floor((nite::getView(nite::RenderTargetGame) + nite::mousePositionAdj(nite::RenderTargetGame)) / nite::Vec2(world.cellsize)));
        if(world.isValid(i)){
            nite::print("[client] "+nite::toStr(i));
            nite::Packet clickOn(Game::PacketType::SV_CLICK_ON);
            clickOn.write(&i, sizeof(i));
            sendPacketFor(this->sv, clickOn);
        }
    }
}

static float lerp(float x1, float x2, float f){
	return  f * x2 + (1.0f - f) * x1;
}

void Game::Client::game(){
    player();
    processIncomPackets();
   
    // send input
    if(nite::getTicks()-lastGameUpdate > gameTickRate){
        nite::Packet pack(Game::PacketType::SV_CLIENT_INPUT);
        auto compat = input.getCompat();
        pack.write(&compat, sizeof(compat));
        pack.write(&pointer.x, sizeof(pointer.x));
        pack.write(&pointer.y, sizeof(pointer.y));
        this->sendPacketFor(this->sv, pack);
        lastGameUpdate = nite::getTicks();
    } 

    for(auto &it : world.objects){
        auto obj = it.second.get();
		if(obj->lastRouteMove > 0 && nite::getTicks()-obj->lastMove > obj->speed){
			auto nstep = obj->route.route[obj->lastRouteMove - 1];
            --obj->lastRouteMove;
            obj->lastMove = nite::getTicks();
            auto diff = nite::Vec2(nstep.x, nstep.y) - obj->position; // make it relative
            world.swapCells(nstep.index, world.toIndex(obj->position));
            obj->position = obj->position + nite::Vec2(diff.x, diff.y);
            obj->nextPosition = obj->position;
		}
        if(obj->speed > 0){
            float dt = ((float)nite::getDelta() / 1000.0f);
            float spd = ((float)obj->speed / 1000.0f);
            if(obj->rPosition.x < obj->nextPosition.x * world.cellsize){
                obj->rPosition.x += ((float)world.cellsize / spd) * dt;
            }
            if(obj->rPosition.x > obj->nextPosition.x * world.cellsize){
                obj->rPosition.x -= ((float)world.cellsize / spd) * dt;
            }  
            if(obj->rPosition.y < obj->nextPosition.y * world.cellsize){
                obj->rPosition.y += ((float)world.cellsize / spd) * dt;
            }
            if(obj->rPosition.y > obj->nextPosition.y * world.cellsize){
                obj->rPosition.y -= ((float)world.cellsize / spd) * dt;
            }                       
        }
    }

    deliverPacketQueue();
    vfx.step();   
}

Game::EntityBase *Game::Client::getEntity(UInt16 id){
    if(id == 0){
        return NULL;
    }
    auto obj = this->world.get(id);
    if(obj == NULL){
        return NULL;
    }
    if(obj->objType != Game::ObjectType::Entity){
        return NULL;
    }
    return static_cast<Game::EntityBase*>(obj);
}

void Game::Client::render(){
    nite::setColor(1.0f, 0.1f, 0.1f, 1.0f);
    nite::setRenderTarget(nite::RenderTargetUI2);
    nite::setDepth(nite::DepthTop);
    debugFont.draw("ping "+nite::toStr(this->ping)+"ms | "+lastChange, nite::getWidth(), 0.0f, 1.0f, 0.0f, 0.0f);
    nite::setColor(1.0f, 1.0f, 1.0f, 1.0f);
    nite::setRenderTarget(nite::RenderTargetGame);
    nite::setDepth(nite::DepthMiddle);
    if(map != NULL){
        auto pos = nite::getView(nite::RenderTargetGame);
        auto sizeAdj = nite::getAdjustedSize();
        auto size = nite::getSize();
        auto sizeDiff = (sizeAdj - size);
        auto sizeDiffH = sizeDiff * nite::Vec2(0.5f);
        // draw tiles within view scope plus targetExcess diff (getAdjustedSize)
        map->draw(nite::Vec2(0.0f), nite::Rect(pos.x - sizeDiffH.x, pos.y - sizeDiffH.y, nite::getWidth() + sizeDiff.x, nite::getHeight() + sizeDiff.y));


        // int wt = nite::round((size.x + sizeDiff.x) / world.gridSpec.x);
        // int ht = nite::round((size.y + sizeDiff.y) / world.gridSpec.y);

        // nite::setDepth(nite::DepthTop);
        // nite::setColor(0.0f, 0.8f, 0.0f, 0.5f);
        // static nite::Texture empty("data/texture/empty.png");
        // for(int x = 0; x < wt; ++x){
        //     for(int y = 0; y < ht; ++y){
        //         float _x = x * world.gridSpec.x;
        //         float _y = y * world.gridSpec.y;
        //         empty.draw(_x + pos.x - sizeDiffH.x, _y + pos.y - sizeDiffH.y, world.gridSpec.x, world.gridSpec.y, 0.0f, 0.0f, 0.0f);
        //     }
        // }

        // nite::Vec2 snap = nite::round(position.x / container->gridSpec.x) * world.cusize;

    }
    for(auto &obj : world.objects){
        obj.second->draw();
    }

    nite::setDepth(nite::DepthTop);
    // hud.update();
    vfx.draw();
    // hud.draw();

    world.renderDebug();
}
