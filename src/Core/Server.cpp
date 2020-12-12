#include <algorithm>

#include "../Engine/Tools/Tools.hpp"
#include "../Engine/Console.hpp"
// #include "../Engine/UI/UI.hpp"

#include "../Game.hpp"
#include "Server.hpp"
#include "Input.hpp"
#include "Object.hpp"
#include "../Entity/Base.hpp"

// static Shared<nite::BaseUIComponent> debug = Shared<nite::BaseUIComponent>(NULL);
static bool debugging = false;
static UInt64 lastDebug = nite::getTicks();

static nite::Console::Result cfCloseServer(Vector<String> params){
    auto core = Game::getGameCoreInstance();
    auto &sv = core->localSv;    
    sv.broadcast("server close down requested remotely...");
    sv.close();
    return nite::Console::Result();
}
static auto cfSvCloseIns = nite::Console::CreateFunction("sv_close", &cfCloseServer, true, true);
static auto cfSvStopIns = nite::Console::CreateFunction("sv_stop", &cfCloseServer, true, true);


static nite::Console::Result cfSimTimescale(Vector<String> params){
    auto core = Game::getGameCoreInstance();
    auto &sv = core->localSv;    
    if(params.size() < 1){
        return nite::Console::Result("not enough parameters(1)", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
    }
    String nts = params[0];
    if(!nite::isNumber(nts)){
        return nite::Console::Result("'"+nts+"' is not a valid parameter", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
    }
    sv.setSimulationProps(sv.gameTickRate, nite::toFloat(nts));
    return nite::Console::Result();
}
static auto cfSimTimescaleIns = nite::Console::CreateFunction("sv_sim_timescale", &cfSimTimescale, true, true);


static nite::Console::Result cfSimTickRate(Vector<String> params){
    auto core = Game::getGameCoreInstance();
    auto &sv = core->localSv;    
    if(params.size() < 1){
        return nite::Console::Result("not enough parameters(1)", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
    }
    String ntr = params[0];
    if(!nite::isNumber(ntr)){
        return nite::Console::Result("'"+ntr+"' is not a valid parameter", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
    }
    sv.setSimulationProps(nite::toInt(ntr), sv.gameTickRate);
    return nite::Console::Result();
}
static auto cfSimTickRateIns = nite::Console::CreateFunction("sv_sim_tickrate", &cfSimTickRate, true, true);



static nite::Console::Result cfDmgEntity(Vector<String> params){
    auto core = Game::getGameCoreInstance();
    auto &sv = core->localSv;    

    if(params.size() == 0){
        return nite::Console::Result("not enough parameters(2)", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
    }
    String amntStr = params[1], idStr = params[0];

    if(!nite::isNumber(amntStr)){
        return nite::Console::Result("'"+amntStr+"' is not a valid parameter", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
    }

    if(!nite::isNumber(idStr)){
        return nite::Console::Result("'"+idStr+"' is not a valid parameter", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
    }    

    Int32 entId = nite::toInt(idStr), amnt = nite::toInt(amntStr);

    auto it = sv.world.objects.find(entId);
    if(it == sv.world.objects.end()){
        return nite::Console::Result("object id '"+idStr+"' doesn't exist", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
    }   
    auto &obj = it->second;

    if(obj->objType != Game::ObjectType::Entity){
        return nite::Console::Result("object id '"+idStr+"' is not an entity", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
    }    
    auto ent = static_cast<Game::EntityBase*>(obj.get());
    Game::DamageInfo dmg;
    dmg.amnt = amnt;
    dmg.dmgtype = Game::DamageType::Magical;
    dmg.elmnt = Game::Element::Neutral;
    dmg.isCrit = false;
    dmg.emitter = 0;
    dmg.receiver = obj->id;
    dmg.truedmg = false;
    dmg.weap = 0;
    ent->damage(dmg);
    return nite::Console::Result();
}
static auto cfDmgEntityIns = nite::Console::CreateFunction("ent_apply_dmg", &cfDmgEntity, true, true);

static nite::Console::Result cfAddEffectEntity(Vector<String> params){
    auto core = Game::getGameCoreInstance();
    auto &sv = core->localSv;    

    if(params.size() == 0){
        return nite::Console::Result("not enough parameters(2)", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
    }
    String entityIdStr = params[0], effectIdStr = params[1];
    
    if(!nite::isNumber(entityIdStr)){
        return nite::Console::Result("'"+entityIdStr+"' is not a valid parameter", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
    }

    if(!nite::isNumber(effectIdStr)){
        return nite::Console::Result("'"+effectIdStr+"' is not a valid parameter", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
    }    

    UInt16 entId = nite::toInt(entityIdStr), effectId = nite::toInt(effectIdStr);

    auto it = sv.world.objects.find(entId);
    if(it == sv.world.objects.end()){
        return nite::Console::Result("object id '"+entityIdStr+"' doesn't exist", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
    }   
    auto &obj = it->second;

    if(obj->objType != Game::ObjectType::Entity){
        return nite::Console::Result("object id '"+entityIdStr+"' is not an entity", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
    }    
    
    auto ent = static_cast<Game::EntityBase*>(obj.get());
    auto effect = Game::getEffect(effectId);

    if(effect.get() == NULL){
        return nite::Console::Result("effect type '"+nite::toStr(effectId)+"' doesn't exist", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
    }

    effect->setup();
    ent->effectStat.add(effect);

    return nite::Console::Result();
}
static auto cfAddEffectEntityIns = nite::Console::CreateFunction("ent_add_effect", &cfAddEffectEntity, true, true);


static nite::Console::Result cfGiveItemEntity(Vector<String> params){
    auto core = Game::getGameCoreInstance();
    auto &sv = core->localSv;    

    if(params.size() < 3){
        return nite::Console::Result("not enough parameters(2)", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
    }
    String entityIdStr = params[0], itemIdStr = params[1], amountStr = params[2];
    
    if(!nite::isNumber(entityIdStr)){
        return nite::Console::Result("'"+entityIdStr+"' is not a valid parameter", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
    }

    if(!nite::isNumber(itemIdStr)){
        return nite::Console::Result("'"+itemIdStr+"' is not a valid parameter", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
    } 

    if(!nite::isNumber(amountStr)){
        return nite::Console::Result("'"+amountStr+"' is not a valid parameter", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
    }          

    UInt16 entId = nite::toInt(entityIdStr), itemId = nite::toInt(itemIdStr), amount = nite::toInt(amountStr);

    auto it = sv.world.objects.find(entId);
    if(it == sv.world.objects.end()){
        return nite::Console::Result("object id '"+entityIdStr+"' doesn't exist", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
    }   
    auto &obj = it->second;

    if(obj->objType != Game::ObjectType::Entity){
        return nite::Console::Result("object id '"+entityIdStr+"' is not an entity", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
    }    
    
    auto ent = static_cast<Game::EntityBase*>(obj.get());
    // This barely takes into account the amountable property
    // TODO: fix this
    // same applies for InventoryStat::add
    auto item = Game::getItem(itemId, amount);

    if(item.get() == NULL){
        return nite::Console::Result("item id '"+nite::toStr(itemId)+"' doesn't exist", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
    }

    ent->invStat.add(item);

    return nite::Console::Result();
}
static auto cfGiveItemEntityIns = nite::Console::CreateFunction("ent_give_item", &cfGiveItemEntity, true, true);



static nite::Console::Result cfRemoveItemEntity(Vector<String> params){
    auto core = Game::getGameCoreInstance();
    auto &sv = core->localSv;    

    if(params.size() < 3){
        return nite::Console::Result("not enough parameters(3)", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
    }
    String entityIdStr = params[0], itemIdStr = params[1], amountStr = params[2];
    UInt16 entId = nite::toInt(entityIdStr), itemId = nite::toInt(itemIdStr), amount = nite::toInt(amountStr);

    auto it = sv.world.objects.find(entId);
    if(it == sv.world.objects.end()){
        return nite::Console::Result("object id '"+entityIdStr+"' doesn't exist", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
    }   
    auto &obj = it->second;

    if(obj->objType != Game::ObjectType::Entity){
        return nite::Console::Result("object id '"+entityIdStr+"' is not an entity", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
    }    
    auto ent = static_cast<Game::EntityBase*>(obj.get());
    auto item = Game::getItem(itemId, 1);
    if(item.get() == NULL){
        return nite::Console::Result("item id '"+nite::toStr(itemId)+"' doesn't exist", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
    }
    ent->invStat.remove(itemId, amount);
    return nite::Console::Result();
}
static auto cfRmItemEntityIns = nite::Console::CreateFunction("ent_rm_item", &cfRemoveItemEntity, true, true);
static auto cfRemoveItemEntityIns = nite::Console::CreateFunction("ent_remove_item", &cfRemoveItemEntity, true, true);


static nite::Console::Result cfEntityStats(Vector<String> params){
    auto core = Game::getGameCoreInstance();
    auto &sv = core->localSv;    

    if(params.size() < 1){
        return nite::Console::Result("not enough parameters(1)", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
    }
    String entityIdStr = params[0];    
    UInt16 entId = nite::toInt(entityIdStr);
    auto ent = sv.getEntity(entId);
    if(ent == NULL){
        return nite::Console::Result("entity id '"+entityIdStr+"' doesn't exist", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
    }   
    ent->printInfo();
    return nite::Console::Result();
}
static auto cfEntityStatsIns = nite::Console::CreateFunction("ent_stats", &cfEntityStats, false, false);


static nite::Console::Result cfEntityAddStat(Vector<String> params){
    auto core = Game::getGameCoreInstance();
    auto &sv = core->localSv;    

    if(params.size() < 3){
        return nite::Console::Result("not enough parameters(3)", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
    }
    String entityIdStr = params[0], stat = params[1], amount = params[2];

    if(!nite::isNumber(entityIdStr)){
        return nite::Console::Result("'"+entityIdStr+"' is not a valid parameter", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
    }
    UInt16 entId = nite::toInt(entityIdStr);
    auto ent = sv.getEntity(entId);
    if(ent == NULL){
        return nite::Console::Result("entity id '"+entityIdStr+"' doesn't exist", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
    }
    UInt8 toAdd;
    if(stat == "str"){
        toAdd = Game::BaseStatType::Strength;    
    }else
    if(stat == "agi"){
        toAdd = Game::BaseStatType::Agility;    
    }else
    if(stat == "dex"){
        toAdd = Game::BaseStatType::Dexterity;    
    }else
    if(stat == "endu"){
        toAdd = Game::BaseStatType::Endurance;    
    }else
    if(stat == "luk"){
        toAdd = Game::BaseStatType::Luck;    
    }else
    if(stat == "intel"){
        toAdd = Game::BaseStatType::Intelligence;    
    }else
    if(stat == "charm"){
        toAdd = Game::BaseStatType::Charisma;            
    }else{
        return nite::Console::Result("stat '"+stat+"' is undefined", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
    }
    // TODO: as is it's technically possible to change other player's stats. fix this
    nite::print(ent->addBaseStat(toAdd, nite::toInt(amount)));
    return nite::Console::Result();
}
static auto cfEntityAddStatIns = nite::Console::CreateFunction("ent_add_stat", &cfEntityAddStat, true, false);



static nite::Console::Result cfEntityLvUp(Vector<String> params){
    auto core = Game::getGameCoreInstance();
    auto &sv = core->localSv;    

    if(params.size() < 2){
        return nite::Console::Result("not enough parameters(2)", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
    }
    String entityIdStr = params[0],amount = params[1];

    if(!nite::isNumber(entityIdStr)){
        return nite::Console::Result("'"+entityIdStr+"' is not a valid parameter", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
    }
    UInt16 entId = nite::toInt(entityIdStr);
    auto ent = sv.getEntity(entId);
    if(ent == NULL){
        return nite::Console::Result("entity id '"+entityIdStr+"' doesn't exist", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
    }
    ent->lvUp(nite::toInt(amount));
    return nite::Console::Result();
}
static auto cfEntityLvUpIns = nite::Console::CreateFunction("ent_lvup", &cfEntityLvUp, true, true);


static nite::Console::Result cfKick(Vector<String> params){
    auto core = Game::getGameCoreInstance();
    auto &sv = core->localSv;    
    if(params.size() == 0){
        return nite::Console::Result("not enough parameters(1)", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
    }    
    // by nickname
    String nick = params[0];
    String reason = params.size() > 1 ? params[1] : "no reason";
    auto *client = sv.getClient(nick);
    if(client == NULL){
        return nite::Console::Result("cannot kick '"+nick+"': doesn't exist", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
    }
    sv.dropClient(client->ip, reason);
    return nite::Console::Result();
}
static auto cfKickIns = nite::Console::CreateFunction("kick", &cfKick, true, true);


Game::Server::Server(){
    this->init = false;
    this->isServer = true;
    lastClientId = nite::randomInt(101, 251);
    setState(Game::ServerState::Off);
    traps.start(this);
}

void Game::Server::setState(unsigned state){
    this->state = state;
    lastState = nite::getTicks();
}

Game::Server::~Server(){

}

Vector<nite::IP_Port> Game::Server::getAllClientsIps(){
    Vector<nite::IP_Port> ips;
    for(auto &it : clients){
        ips.push_back(it.second.ip);
    }
    return ips;    
}

Vector<UInt32> Game::Server::getAllClientsAcks(){
    Vector<UInt32> acks;
    for(auto &it : clients){
        acks.push_back(++it.second.svAck);
    }
    return acks;
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
    ft.listen(nite::NetworkDefaultFileTransferPort);
    nite::print("[server] started server '"+name+"' | max clients "+nite::toStr(this->maxClients)+" | listening at "+nite::toStr(port));
    nite::print("[server] waiting for clients...");
    setSimulationProps(32, 1.0f);
    this->init = true;
    setState(Game::ServerState::Idle);

    if(debugging){
        // debug = nite::UI::build("./data/ui/debug_window.json");
        auto json = Jzon::object();
        json.add("type", "text");
        json.add("fontColor", "#000000");
        json.add("fontSize", "16");
        json.add("text", "debug");
        // debug->add(nite::UI::build(json));
    }
}

Game::SvClient *Game::Server::getClient(UInt64 uid){
    return clients.count(uid) > 0 ? &clients[uid] : NULL;
}

Game::SvClient *Game::Server::getClient(const String &nick){
    for(auto _cl : clients){
        if(_cl.second.nickname == nick){
            return &clients[_cl.first];
        }
    }
    return NULL;
}

Game::SvClient *Game::Server::getClientByIp(const nite::IP_Port &ip){
    for(auto _cl : clients){
        if(_cl.second.ip.isSame(ip)){
            return &clients[_cl.first];
        }
    }
    return NULL;
}

Game::SvClient *Game::Server::getClientByEntityId(UInt16 entityId){
    for(auto &it : players){
        if(it.second == entityId){
            return getClient(it.first);
        }
    }
    return NULL;
}

void Game::Server::dropClient(const nite::IP_Port &ip, String reason){
    auto cl = this->getClientByIp(ip);
    if(cl == NULL){
        nite::print("[server] cannot drop client "+(String)ip+": it's not connected");
        return;
    }
    if(cl->entityId != 0){
        destroy(cl->entityId); // destroy player's entity
    }
    nite::Packet drop(Game::PacketType::SV_CLIENT_DROP);
    drop.write(reason);
    sock.send(cl->ip, drop);
    removeClient(cl->ip);
    // notify client dropped to others
    nite::Packet noti;
    noti.setHeader(Game::PacketType::SV_NOTI_CLIENT_DROP);
    noti.write(&cl->clientId, sizeof(UInt64));
    noti.write(reason);
    sendPersPacketForMany(getAllClientsIps(), noti, getAllClientsAcks());
}

void Game::Server::sendRemoteCmdMsg(UInt64 uid, const String &msg, const nite::Color &color){
    auto cl = this->getClient(uid);
    if(cl == NULL){
        nite::print("[server] cannot send remote cmd msg to client "+nite::toStr(uid)+": it's not connected");
        return;
    }
    nite::Packet msgPacket;
    msgPacket.setHeader(Game::PacketType::SV_REMOTE_CMD_MSG);
    msgPacket.setOrder(++cl->lastSentOrder);
    msgPacket.setAck(++cl->svAck);
    msgPacket.write(msg);
    msgPacket.write(&color.r, sizeof(float));
    msgPacket.write(&color.g, sizeof(float));
    msgPacket.write(&color.b, sizeof(float));
    sendPersPacketFor(cl->ip, msgPacket, ++cl->svAck);
}

void Game::Server::dropClient(const nite::IP_Port &ip){
    dropClient(ip, "no reason");
}

void Game::Server::removeClient(const nite::IP_Port &ip){
    dropPersFor(ip);
    auto client = getClientByIp(ip);
    if(!client){
        return;
    }
    auto it = clients.find(client->clientId);
    clients.erase(it);
}

void Game::Server::close(){
    if(!init){
        return;
    }
    setState(Game::ServerState::ShuttingDown);
    broadcast("server is closing down");
    nite::print("[server] closing server down, dropping clients...");
    Vector<nite::IP_Port> ips;
    for(auto cl : clients){
        ips.push_back(cl.second.ip);
    }
    for(int i = 0; i < ips.size(); ++i){
        dropClient(ips[i], "server closing down");
    }
    clear();
    broadcast("[server] closed server down");
}

void  Game::Server::setSimulationProps(UInt64 tickrate, float ts){
    this->gameTickRate = tickrate;
    this->gameTimescale = ts;
    world.tickrate = gameTickRate;
    world.timescale = gameTimescale;   
    if(init){
        nite::Packet simProps(Game::PacketType::SV_UPDATE_WORLD_SIMULATION_PROPS);
        simProps.write(&gameTimescale, sizeof(gameTimescale));
        simProps.write(&gameTickRate, sizeof(gameTickRate));
        sendPersPacketForMany(getAllClientsIps(), simProps, getAllClientsAcks());
    }
}

void Game::Server::clear(){
    this->init = false;
    sock.close();
    world.clearWallMasks();
    world.clearGhostMasks();
    ft.clear();
    deliveries.clear();
    clients.clear();
    players.clear();
    maps.clear();
    tilesets.clear();
    world.clear();
    world.tickrate = gameTickRate;
    world.timescale = gameTimescale;
    setState(Game::ServerState::Off);
    // if(debugging && debug.get() != NULL){
    //     static_cast<nite::WindowUI*>(debug.get())->close();
    // }    
    for(int i = 0; i < maps.size(); ++i){
        maps[i]->unload();
    }
    maps.clear();
    // clear generated folder
    Vector<String> cleanMaps;
    nite::fileFind("./data/map/generated/", nite::Find::File, ".json", false, true, cleanMaps);
    auto indexer = nite::getIndexer();
    for(int i = 0; i < cleanMaps.size(); ++i){
        nite::removeFile(cleanMaps[i]);
        indexer->removeByPath(cleanMaps[i]);
    }      
}

void Game::Server::update(){
    if(!init){
        return;
    }
    nite::IP_Port sender;
    nite::Packet handler;
    if(sock.recv(sender, handler) > 0){
        auto client = this->getClientByIp(sender);
        bool isLast = client && handler.getOrder() > client->lastRecvOrder;
        if(client && isLast){
            client->lastPacketTimeout = nite::getTicks();
            client->lastPacket = handler;
            client->lastRecvOrder = handler.getOrder();
        }
        // respond immediate-packets
        switch(handler.getHeader()){
            /*
                SV_PONG
            */      
            case Game::PacketType::SV_PONG: {
                if(!client || !isLast){
                    break;
                }
                client->ping = nite::getTicks() - client->lastPing; 
            } break; 
            /*
                SV_PING
            */             
            case Game::PacketType::SV_PING: {
                if(!client || !isLast){
                    break;
                }
                nite::Packet pong;
                pong.setHeader(Game::PacketType::SV_PONG);
                pong.setOrder(++client->lastSentOrder);
                sock.send(client->ip, pong);                
            } break;   
            /*
                SV_CONNECT_REQUEST
            */
            case Game::PacketType::SV_CONNECT_REQUEST : {
                nite::print("[server] incoming connection...");
                if(clients.size() == maxClients){
                    static const String msg = "server is full";
                    nite::Packet reject(Game::PacketType::SV_CONNECT_REJECT);
                    reject.setOrder(0);
                    reject.write(msg);
                    nite::print("[server] rejected client "+sender.str()+": "+msg);
                    sock.send(sender, reject); 
                    break;
                }
                // TODO: add option to allow campaigns to let players join midgame
                if(state == Game::ServerState::InGame){
                    nite::Packet reject(Game::PacketType::SV_CONNECT_REJECT);
                    String msg = "game already started";
                    reject.setOrder(0);
                    reject.write(msg);
                    nite::print("[server] rejected client "+sender.str()+": "+msg);
                    sock.send(sender, reject); 
                    break;
                }
                UInt32 clVer;
                String nick;
                handler.read(&clVer, sizeof(UInt32));
                handler.read(nick);
                Game::SvClient nclient(nick, ++lastClientId, sender);
                clients[nclient.clientId] = nclient;
                auto client = &clients[nclient.clientId];
                nite::print("[server] accepted clientId "+sender.str()+" | nickname '"+nick+"'");
                if(clients.size() > 0 && state == Game::ServerState::Idle){
                    nite::print("[server] awaiting for client to load...");
                    setState(Game::ServerState::Waiting);
                }
                nite::Packet accepted(Game::PacketType::SV_CONNECT_ACCEPT);
                accepted.write(&client->clientId, sizeof(client->clientId));
                accepted.write(this->name);                
                sendPersPacketFor(sender, accepted, ++client->svAck);
                bindOnAckFor(client->svAck, [&](nite::SmallPacket &payload, nite::IP_Port &ip){
                    auto client = getClientByIp(ip);
                    if(client == NULL){
                        nite::print("[server] SV_CONNECT_REQUEST -> SV_CONNECT_ACCEPT: fatal failure: client doesn't exist anymore?");
                        return;
                    }
                    nite::Packet await(Game::PacketType::SV_AWAIT_CLIENT_LOAD);
                    await.write(map->hash);
                    sendPersPacketFor(ip, await, ++client->svAck);
                    bindOnAckFor(client->svAck, [&](nite::SmallPacket &payload, nite::IP_Port &ip){
                        auto client = getClientByIp(ip);
                        nite::Packet notify(Game::PacketType::SV_CLIENT_JOIN);
                        notify.write(&client->clientId, sizeof(client->clientId));
                        notify.write(client->nickname);
                        sendPersPacketForMany(getAllClientsIps(), notify, getAllClientsAcks());
                        sendPlayerList(client->clientId);                            
                    });
                });
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
                nite::print("[server] disconnected client "+sender.str()+": "+reason);
                removeClient(sender);   
                // notify client dropped to others
                nite::Packet noti;
                noti.setHeader(Game::PacketType::SV_NOTI_CLIENT_DROP);
                noti.write(&client->clientId, sizeof(UInt64));
                noti.write(reason);
                sendPersPacketForMany(getAllClientsIps(), noti, getAllClientsAcks());                             
            } break; 
            default: {
                rcvPackets.push_back(handler);
            };

        }        
    }

    // refresh player info
    // if(nite::getTicks()-lastPlayerInfoSent > 1000){
    //     lastPlayerInfoSent = nite::getTicks();
    //     for(auto cl : clients){
    //         sendPlayerList(cl.first);
    //     }        
    // }

    // timeout
    Vector<nite::IP_Port> timedoutClients;
    for(auto cl : clients){
        if(nite::getTicks()-cl.second.lastPacketTimeout > ClientTimeout){
            timedoutClients.push_back(cl.second.ip);
        }
    }
    for(int i = 0; i < timedoutClients.size(); ++i){
        dropClient(timedoutClients[i], "timeout");
        nite::print("[server] dropped clientId "+timedoutClients[i].str()+": timeout");
    }

    // ping
    for(auto &cl : clients){
        auto &client = cl.second;
        if(nite::getTicks()-client.lastPing > 1000){
            client.lastPing = nite::getTicks();
            nite::Packet ping(Game::PacketType::SV_PING);
            ping.setOrder(++client.lastSentOrder);
            sock.send(client.ip, ping);
        }
    }
    game();
}

Game::EntityBase *Game::Server::getEntity(UInt16 id){
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

void Game::Server::broadcast(const String &message){
    if(!init){
        return;
    }    
    if(message.length() == 0){
        nite::print("[server] cannot broadcast an empty message");
        return;
    }
    nite::Packet msg(Game::PacketType::SV_BROADCAST_MESSAGE);
    msg.write(message);
    nite::print("[BROADCAST] "+message);
    // persSendAll(msg, 750, 4);
    sendPersPacketForMany(getAllClientsIps(), msg, getAllClientsAcks());
}

void Game::Server::processIncomPackets(){
    if(!init) return;
    // process rcvPackets
    std::function<void(nite::Packet &handler, bool ignoreOrder)> processPacket = [&](nite::Packet &handler, bool ignoreOrder){
        auto sender = handler.sender;
        auto client = this->getClientByIp(sender);
        bool isLast = client && handler.getOrder() > client->lastRecvOrder;
        if(client && isLast && !ignoreOrder){
            client->lastPacketTimeout = nite::getTicks();
            client->lastPacket = handler;
            client->lastRecvOrder = handler.getOrder();
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
                if(!client){
                    break;
                }                    
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
                    holder.sender = client->ip;
                    processPacket(holder, true);
                }
            } break;
            /*
                SV_ACK
            */            
            case Game::PacketType::SV_ACK: {
                ack(handler);
            } break; 
            /*
                SV_CHAT_MESSAGE
            */             
            case Game::PacketType::SV_CHAT_MESSAGE: {
                if(!client || !isLast){
                    break;
                }
                sendAck(client->ip, handler.getAck());
                UInt64 uid;
                String msg;
                handler.read(&uid, sizeof(UInt64));
                handler.read(msg);
                nite::print("SERVER ["+client->nickname+"] "+msg);
                sendPersPacketForMany(getAllClientsIps(), handler, getAllClientsAcks());
            } break; 
            /*
                SV_CLIENT_INPUT
            */
            case Game::PacketType::SV_CLIENT_INPUT: {
                if(!client || !isLast){
                    break;
                }
                auto ent = getEntity(client->entityId);
                if(ent != NULL){
                    UInt16 compat;
                    float mx, my;
                    handler.read(&compat, sizeof(compat));
                    handler.read(&mx, sizeof(mx));
                    handler.read(&my, sizeof(my));
                    ent->input.loadCompat(compat);
                    if(ent->input.mpos.x != mx || ent->input.mpos.y != my){
                        ent->input.mpos.set(mx, my);
                    }
                }
            } break;       
            /*
                SV_RCON
            */
            case Game::PacketType::SV_RCON: {
                if(!client || !isLast){
                    break;
                }
                sendAck(client->ip, handler.getAck());
                String pwd;
                handler.read(pwd);
                static const String hash = nite::hashString("pwd");
                if(pwd == hash){ // TODO: temporary
                    client->role = Game::SvClientRole::Admin;
                    nite::print("rcond client id "+nite::toStr(client->clientId)+" as admin");
                    sendRemoteCmdMsg(client->clientId, "you're now rcond as admin", nite::Color(0.16f, 0.85f, 0.12f, 1.0f));
                }else{
                    client->role = Game::SvClientRole::Admin;
                    nite::print("client id "+nite::toStr(client->clientId)+" failed to rcon");
                    sendRemoteCmdMsg(client->clientId, "wrong password", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
                }
            } break;               
            /*
                SV_REMOTE_CMD_EXEC
            */
            case Game::PacketType::SV_REMOTE_CMD_EXEC: {
                if(!client || !isLast){
                    break;
                }
                sendAck(client->ip, handler.getAck());
                String cmd;
                handler.read(cmd);
                auto result = nite::Console::interpret(cmd, false, true, client->role == Game::SvClientRole::Admin);
                sendRemoteCmdMsg(client->clientId, result.msg, result.color);
            } break;  
            /*
                SV_CLIENT_LOAD_READY
            */             
            case Game::PacketType::SV_CLIENT_LOAD_READY: {
                if(!client || !isLast){
                    break;
                }
                sendAck(client->ip, handler.getAck());
                client->ready = true;
            } break;             
            /*
                SV_ENTITY_USE_SKILL_ITEM
            */             
            case Game::PacketType::SV_ENTITY_USE_SKILL_ITEM: {
                if(!client || !isLast){
                    break;
                }
                sendAck(client->ip, handler.getAck());
                UInt16 userId, targetId;                
                UInt8 type;
                UInt32 id;
                float x, y;
                handler.read(&userId, sizeof(userId));
                handler.read(&type, sizeof(type));
                handler.read(&id, sizeof(id));
                handler.read(&targetId, sizeof(targetId));
                handler.read(&x, sizeof(x));
                handler.read(&y, sizeof(y));
                auto user = this->getEntity(userId);
                if(user == NULL){
                    nite::print("[server] failed to invokeUse for entity id '"+nite::toStr(userId)+"': doesn't exist");
                    break;
                }
                switch(type){
                    case ActionableType::Skill:
                    case ActionableType::Item: {
                        user->invokeUse(targetId, type, id, x, y);
                    } break;
                    default: {
                        nite::print("[server] entity id '"+nite::toStr(userId)+"' invoked usage of undefined type '"+nite::toStr(type)+"'");
                    } break;
                }
            } break;         
            /* 
                UNKNOWN
            */
            default: {
                if(!client){
                    break;
                } 
                nite::print("[server] unknown packet type '"+nite::toStr(handler.getHeader())+"'");
            } break;  
        }
    };

    for(int i = 0; i < rcvPackets.size(); ++i){
        processPacket(rcvPackets[i], false);
    }
    rcvPackets.clear();    
}

void Game::Server::checkGameStatus(){
    if(!init) return;
    // players joined. wait for them to be ready and then start the game
    if(nite::getTicks()-lastState > 5*1000 && state == Game::ServerState::Waiting){
        int n = 0;
        for(auto &cl : clients){
            if(cl.second.ready){
                ++n;
            }
        }
        if(nite::getTicks()-lastState > 30 * 1000 && n > 1 || n == clients.size()){ // game is going to start in 30 seconds regardless when there's at least one client ready
            start();
        }
    }

    // everyone left
    if(clients.size() == 0 && nite::getTicks()-lastState > 5000  && (state == Game::ServerState::InGame || state == Game::ServerState::GameOver)){
        nite::print("[server] no players in the game. closing down server...");
        close();
    }

    // everyone died
    if(clients.size() > 0 && state == Game::ServerState::InGame){
        int allDead = 0;
        for(auto &cl : clients){
            auto it = world.objects.find(cl.second.entityId);
            if(it == world.objects.end()){
                continue;
            }
            if(it->second->objType != Game::ObjectType::Entity){
                continue;
            }
            auto ent = static_cast<Game::EntityBase*>(it->second.get());
            if(ent->healthStat.dead){
                ++allDead;
            }
        }
        if(allDead == clients.size()){
            // TODO: show game over on the screen
            nite::print("[server] Game Over. Restarting in 8 seconds");
            setState(Game::ServerState::GameOver);
            // notify clients
            for(auto &cl : clients){
                notifyGameOver(cl.second.clientId);
            }
        }
    }

    // game over and restart
    if(state == Game::ServerState::GameOver && nite::getTicks()-lastState > 8000){
        restart();
    }  
}

void Game::Server::deliverPacketQueue(){
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
            handle.setOrder(++client.lastSentOrder);
            sock.send(client.ip, handle);
        }
    }
    packetQueue.clear();  
}

void Game::Server::game(){
    if(!init) return;

    // if(debugging && nite::getTicks()-lastDebug > 1000){
    //     auto text = static_cast<nite::TextUI*>(debug->children[0].get());
    //     lastDebug = nite::getTicks();
    //     String composed = "remaining deliveries: "+nite::toStr(deliveries.size())+" | ";
    //     for(int i = 0; i < deliveries.size(); ++i){
    //         composed += " "+nite::toStr(deliveries[i].packet.getHeader());
    //     }        
    //     text->setText(composed);
    // }    


    checkGameStatus();
    processIncomPackets();


    // prepare delta for delivery
    auto prepareDelta = [&](){
        Vector<UInt16> objUpdate;
        for(auto &it : this->world.objects){
            auto obj = it.second.get();
            if(obj->destroyed){
                continue;
            }
            bool animUpd = hasIssuedDeltaStateUpdate(DeltaUpdateType::ANIMATION, obj->deltaUpdates) && (obj->objType == ObjectType::Entity);
            bool phsyUpd = hasIssuedDeltaStateUpdate(DeltaUpdateType::PHYSICS, obj->deltaUpdates);
            if(!animUpd && !phsyUpd){
                continue;
            }
            objUpdate.push_back(obj->id);
        }   
        if(objUpdate.size() == 0){
            return;
        }
        nite::Packet delta(PacketType::SV_UPDATE_OBJECT_STATE);
        UInt8 n = objUpdate.size();
        delta.write(&n, sizeof(n));
        for(int i = 0; i < objUpdate.size(); ++i){
            auto obj = world.get(objUpdate[i]);
            delta.write(&obj->id, sizeof(obj->id));
            delta.write(&obj->deltaUpdates, sizeof(obj->deltaUpdates));
            // ANIMATION
            if(hasIssuedDeltaStateUpdate(DeltaUpdateType::ANIMATION, obj->deltaUpdates) && (obj->objType == ObjectType::Entity)){
                auto ent = static_cast<Game::EntityBase*>(obj);
                delta.write(&ent->faceDirection, sizeof(ent->faceDirection));
                delta.write(&ent->pointingAt.x, sizeof(ent->pointingAt.x));
                delta.write(&ent->pointingAt.y, sizeof(ent->pointingAt.y));
                for(int j = 0; j < EntityStateSlot::total; ++j){
                    delta.write(&ent->state[j], sizeof(UInt8));
                    delta.write(&ent->stNum[j], sizeof(UInt8));
                    delta.write(&ent->lastExpectedTime[j], sizeof(ent->lastExpectedTime[j]));
                }                    
            }
            // PHYSICS
            if(hasIssuedDeltaStateUpdate(DeltaUpdateType::PHYSICS, obj->deltaUpdates)){
                delta.write(&obj->direction, sizeof(obj->direction));
                delta.write(&obj->speed, sizeof(obj->speed));                    
                delta.write(&obj->position.x, sizeof(obj->position.x));
                delta.write(&obj->position.y, sizeof(obj->position.y));                                    
            }  
            obj->clearDeltaUpdates();               
        }            
        for(auto &it : clients){
            delta.setOrder(currentDelta);
            sendPacketFor(it.second.ip, delta);
        }
    };
        
    // run delta
    if(nite::getTicks()-lastGameUpdate > gameTickRate){
        world.update();
        ++currentDelta;
        prepareDelta();
        lastGameUpdate = nite::getTicks();    
    } 
      
    deliverPacketQueue();
}

void Game::Server::createPlayersOnStart(UInt16 initialHeader){
    // set simulation properties to have them sent to clients
    setSimulationProps(gameTickRate, gameTimescale);

    auto me = this;
    // and now some chained-deliveries craziness... (apologies in advance)

    // [0] bind on the initialHeader before we start creating players and asigning
    me->bindOnAckForHeader(initialHeader, [me](nite::SmallPacket &pck, nite::IP_Port &ip){
        auto cl = me->getClientByIp(ip);
        if(cl == NULL){
            nite::print("[server] failed to notify clients their respective entity. ip was not found in the list");
            return;
        }

        float startx = me->map->startCell.x + nite::randomInt(-50, 50);
        float starty = me->map->startCell.y + nite::randomInt(-50, 50);    

        me->createPlayer(cl->clientId, 1, startx, starty);
        // [1] notify clients of their respective owners
        me->bindOnAckForHeader(Game::PacketType::SV_CREATE_OBJECT, [me](nite::SmallPacket &pck, nite::IP_Port &ip){   
            auto cl = me->getClientByIp(ip);
            if(cl == NULL){
                nite::print("[server] failed to notify clients their respective entity. ip was not found in the list");
                return;
            }
            nite::Packet noti(Game::PacketType::SV_NOTI_ENTITY_OWNER);
            noti.write(&cl->entityId, sizeof(UInt16));
            me->sendPersPacketFor(cl->ip, noti, ++cl->svAck);
            // [2] send respective clients their default skill list
            me->bindOnAckForHeader(Game::PacketType::SV_NOTI_ENTITY_OWNER, [me](nite::SmallPacket &pck, nite::IP_Port &ip){   
                auto cl = me->getClientByIp(ip);
                if(cl == NULL){
                    nite::print("[server] failed to send skill list. ip was not found in the list");
                    return;
                }
                me->sendSkillList(cl->clientId, cl->entityId);
                // [3] send respective clients their default keybinds
                me->bindOnAckForHeader(Game::PacketType::SV_SET_ENTITY_SKILLS, [me](nite::SmallPacket &pck, nite::IP_Port &ip){  
                    auto cl = me->getClientByIp(ip);
                    if(cl == NULL){
                        nite::print("[server] failed to send default keybinds: ip was not found in the list");
                        return;
                    }
                    auto itent = me->world.objects.find(cl->entityId);
                    if(itent == me->world.objects.end()){
                        nite::print("[server] failed to send default keybinds: ent id "+nite::toStr(cl->entityId)+" doesn't exist");
                        return;
                    }
                    auto ent = static_cast<Game::EntityBase*>(itent->second.get()); // we're gonna assume this is indeed an entity
                    auto &sklst = ent->skillStat.skills;
                    UInt8 skamnt = 5;
                    nite::Packet packet(Game::PacketType::SV_SET_ENTITY_ACTIONABLES);
                    packet.write(&ent->id, sizeof(UInt16));
                    packet.write(&skamnt, sizeof(UInt8));
                    auto writeSkill = [&](UInt32 skilId, UInt8 slot){
                        static const UInt8 sktype = Game::ActionableType::Skill;
                        packet.write(&slot, sizeof(slot));
                        packet.write(&sktype, sizeof(UInt8));
                        packet.write(&skilId, sizeof(UInt32));
                    };
                    writeSkill(Game::SkillList::BA_ATTACK, 5);
                    writeSkill(Game::SkillList::BA_PARRY, 6);
                    writeSkill(Game::SkillList::BA_BASH, 0);
                    writeSkill(Game::SkillList::BA_DASH, 1);
                    writeSkill(Game::SkillList::BA_FIRST_AID, 2);
                    me->sendPersPacketFor(cl->ip, packet, ++cl->svAck);
                });                    
            });             
        });             
    }); 

    float startx = me->map->startCell.x + nite::randomInt(-50, 50);
    float starty = me->map->startCell.y + nite::randomInt(-50, 50);   

    // auto objMob = createMob(Game::ObjectSig::MobHumanoid, 10, startx, starty); 
    // auto mob = static_cast<Game::EntityBase*>(objMob.get());
    // mob->aidriver.add(Shared<Game::AI::DumbassBehavior>(new Game::AI::DumbassBehavior()));
}

void Game::Server::restart(){
    setState(Game::ServerState::InGame);
    // clean up map
    // TODO
    // remove players
    for(auto &cl : clients){
        cl.second.entityId = 0;
    }
    players.clear();
    // clean up world
    setCurrentMap(this->maps[0]);
    nite::Packet restart(Game::PacketType::SV_SET_GAME_RESTART);
    // persSendAll(restart, 1000, -1);    
    sendPersPacketForMany(getAllClientsIps(), restart, getAllClientsAcks());
    createPlayersOnStart(Game::PacketType::SV_SET_GAME_RESTART); 
}

void Game::Server::start(){
    setState(Game::ServerState::InGame);
    nite::Packet start(Game::PacketType::SV_SET_GAME_START);
    sendPersPacketForMany(getAllClientsIps(), start, getAllClientsAcks()); 
    createPlayersOnStart(Game::PacketType::SV_SET_GAME_START);
}

void Game::Server::setupGame(const String &name, int maxClients, int maps){
    if(maps <= 0){
        maps = 1;
    }
    if(maxClients <= 0){
        maxClients = 4;
    }  
    nite::print("[server] setting up game for "+nite::toStr(maxClients)+" player(s) | "+nite::toStr(maps)+" map(s)");
    //TODO: move/remove this. we're meant to tell this method what tile source to use
    Game::RING::TileSource src("data/tileset/alp_ruins.json");
    auto indexer = nite::getIndexer();
    // build maps
    for(int i = 0; i < maps; ++i){
        auto bp = Shared<Game::RING::Blueprint>(new Game::RING::Blueprint());
        bp->generate(35, 35);
        auto map = Game::RING::generateMap(bp, src, 4);
        String fname = "./data/map/generated/"+map->hash+".json";
        map->exportToJson(fname);
        auto indexed = indexer->indexFile(fname);
        if(indexed == NULL){
            nite::print("failed to index recently generated map file: full storage maybe?");
            continue;
        }
        map->hash = indexed->hash; // we'll replace the temporary hash generated by ring with the actual hash of the file
        this->maps.push_back(map);
    }
    setCurrentMap(this->maps[0]);
    listen(name, maxClients, nite::NetworkDefaultPort);
}

void Game::Server::addItem(UInt16 entityId, Shared<Game::ItemBase> item){
    auto failmsg = "[server] failed to add item for entity id "+nite::toStr(entityId)+": ";
    auto it = world.objects.find(entityId);
    if(it == world.objects.end()){
        nite::print(failmsg+"it doesn't exist");
        return;
    }
    if(auto ent = dynamic_cast<Game::EntityBase*>(it->second.get())){
        ent->invStat.add(item);      
    }else{
        nite::print(failmsg+" it's not an entity");
    }
}

void Game::Server::removeItem(UInt16 entityId, UInt16 slotId, UInt16 amnt){
    auto failmsg = "[server] failed to remove item for entity id "+nite::toStr(entityId)+": ";
    auto it = world.objects.find(entityId);
    if(it == world.objects.end()){
        nite::print(failmsg+"it doesn't exist");
        return;
    }
    if(auto ent = dynamic_cast<Game::EntityBase*>(it->second.get())){
        auto ititem = ent->invStat.carry.find(slotId);
        if(ititem ==  ent->invStat.carry.end()){
            nite::print(failmsg+"it's not in its inventory");
            return;
        }
        auto item = ent->invStat.carry[ititem->first];
        ent->invStat.removeBySlotId(slotId, amnt);
    }else{
        nite::print(failmsg+" it's not an entity");
    }    
}

void Game::Server::addSkill(UInt16 entityId, UInt16 skillId, UInt8 lv){
    auto failmsg = "[server] failed to add skill for entity id "+nite::toStr(entityId)+": ";
    auto it = world.objects.find(entityId);
    if(it == world.objects.end()){
        nite::print(failmsg+"it doesn't exist");
        return;
    }
    auto sk = getSkill(skillId, lv);
    if(sk.get() == NULL){
        nite::print(failmsg+"skill id "+nite::toStr(skillId)+" doesn't exist");
        return;
    }
    if(auto ent = dynamic_cast<Game::EntityBase*>(it->second.get())){
        ent->skillStat.add(sk);  
    }else{
        nite::print(failmsg+" it's not an entity");
    }
}

void Game::Server::removeSkill(UInt16 entityId, UInt16 skillId){
    auto failmsg = "[server] failed to remove skill for entity id "+nite::toStr(entityId)+": ";
    auto it = world.objects.find(entityId);
    if(it == world.objects.end()){
        nite::print(failmsg+"it doesn't exist");
        return;
    }
    auto sk = getSkill(skillId, 0);
    if(sk.get() == NULL){
        nite::print(failmsg+"skill id "+nite::toStr(skillId)+" doesn't exist");
        return;
    }
    if(auto ent = dynamic_cast<Game::EntityBase*>(it->second.get())){
        ent->skillStat.remove(skillId);       
    }else{
        nite::print(failmsg+" it's not an entity");
    }    
}

void Game::Server::addEffect(UInt16 entityId, Shared<Game::Effect> &eff){
    auto failmsg = "[server] failed to add effect for entity id "+nite::toStr(entityId)+": ";
    auto it = world.objects.find(entityId);
    if(it == world.objects.end()){
        nite::print(failmsg+"it doesn't exist");
        return;
    }
    if(auto ent = dynamic_cast<Game::EntityBase*>(it->second.get())){
        ent->effectStat.add(eff);        
    }else{
        nite::print(failmsg+" it's not an entity");
    }
}

void Game::Server::removeEffect(UInt16 entityId, UInt16 insId){
    auto failmsg = "[server] failed to remove effect for entity id "+nite::toStr(entityId)+": ";
    auto it = world.objects.find(entityId);
    if(it == world.objects.end()){
        nite::print(failmsg+"it doesn't exist");
        return;
    }
    if(auto ent = dynamic_cast<Game::EntityBase*>(it->second.get())){
        if(!ent->effectStat.isOn(insId)){
            nite::print(failmsg+" entity doesn't have this effect insId "+nite::toStr(insId));
            return;
        }
        ent->effectStat.remove(insId);     
    }else{
        nite::print(failmsg+" it's not an entity");
    }
}

void Game::Server::notifyAddItem(UInt64 uid, UInt16 itemId, UInt16 slotId, UInt16 qty){
    auto cl = getClient(uid);
    if(cl == NULL){
        return;
    }
    auto ent = getEntity(cl->entityId);
    if(ent == NULL){
        nite::print("notifyAddItem: failed to find entity by id '"+nite::toStr(cl->entityId)+"'");
        return;
    }
    auto item = ent->invStat.get(slotId);
    if(item.get() == NULL){
        nite::print("notifyAddItem: failed to find item by slotId '"+nite::toStr(slotId)+"'");
        return;
    }
    nite::Packet packet(Game::PacketType::SV_ADD_ITEM);
    packet.write(&cl->entityId, sizeof(cl->entityId));
    packet.write(&itemId, sizeof(itemId));
    packet.write(&slotId, sizeof(slotId));
    packet.write(&qty, sizeof(qty));
    packet.write(&item->upgradelv, sizeof(item->upgradelv));
    for(int i = 0; i < GAME_ITEM_MAX_COMPOUND_SLOTS; ++i){
        packet.write(&item->compound[i], sizeof(item->compound[i]));
    } 
    sendPersPacketFor(cl->ip, packet, ++cl->svAck);
}

void Game::Server::notifyRemoveItem(UInt64 uid, UInt16 itemId, UInt16 slotId, UInt16 qty){
    auto cl = getClient(uid);
    if(cl == NULL){
        return;
    }
    nite::Packet packet(Game::PacketType::SV_REMOVE_ITEM);
    packet.write(&cl->entityId, sizeof(cl->entityId));
    packet.write(&itemId, sizeof(itemId));
    packet.write(&slotId, sizeof(slotId));
    packet.write(&qty, sizeof(qty));
    sendPersPacketFor(cl->ip, packet, ++cl->svAck);
}

void Game::Server::notifyAddSkill(UInt64 uid, UInt16 skillId, UInt8 lv){
    auto cl = getClient(uid);
    if(cl == NULL){
        return;
    }
    nite::Packet packet(Game::PacketType::SV_ADD_ENTITY_SKILL);      
    packet.write(&cl->entityId, sizeof(cl->entityId));
    packet.write(&skillId, sizeof(skillId));
    packet.write(&lv, sizeof(lv));
    sendPersPacketFor(cl->ip, packet, ++cl->svAck);
}

void Game::Server::notifyRemoveSkill(UInt64 uid, UInt16 skillId){
    auto cl = getClient(uid);
    nite::Packet packet(Game::PacketType::SV_REMOVE_ENTITY_SKILL);
    packet.write(&cl->entityId, sizeof(cl->entityId));
    packet.write(&skillId, sizeof(skillId));
    sendPersPacketFor(cl->ip, packet, ++cl->svAck);
}

void Game::Server::notifyUpdateEquipSlots(UInt64 uid){
    auto cl = getClient(uid);
    if(cl == NULL){
        return;
    }    
    auto ent = getEntity(cl->entityId);
    if(ent == NULL){
        return;
    }    
    nite::Packet packet(Game::PacketType::SV_UPDATE_ENTITY_INVENTORY_SLOTS);
    packet.write(&cl->entityId, sizeof(cl->entityId));    
    ent->writeInvSlotsState(packet);
    sendPersPacketFor(cl->ip, packet, ++cl->svAck);
}

void Game::Server::notifyUpdateInvList(UInt64 uid){
    auto cl = getClient(uid);
    if(cl == NULL){
        return;
    }    
    auto ent = getEntity(cl->entityId);
    if(ent == NULL){
        return;
    }
    nite::Packet packet(Game::PacketType::SV_UPDATE_ENTITY_INVENTORY_CARRY);
    packet.write(&cl->entityId, sizeof(cl->entityId));   
    ent->writeInvListState(packet);
    ent->writeInvSlotsState(packet);
    sendPersPacketFor(cl->ip, packet, ++cl->svAck);
}

void Game::Server::notifyAddEffect(UInt64 uid, UInt16 type, UInt16 insId){
    String msg = "failed to notify add effect for client uid "+nite::toStr(uid)+": ";
    auto cl = getClient(uid);
    if(cl == NULL){
        nite::print(msg+"it doesn't exist");
        return;
    }
    auto it = world.objects.find(cl->entityId);
    if(it == world.objects.end()){
        nite::print(msg+"doesn't have an active entity");
        return;
    }
    auto ent = static_cast<Game::EntityBase*>(world.objects[it->first].get());
    auto itef = ent->effectStat.effects.find(insId);
    if(itef == ent->effectStat.effects.end()){
        nite::print(msg+"doesn't have an effect insId "+nite::toStr(insId));
        return;
    }
    auto ef = ent->effectStat.effects[insId];
    nite::Packet packet(Game::PacketType::SV_ADD_EFFECT);
    packet.write(&cl->entityId, sizeof(cl->entityId));
    packet.write(&type, sizeof(type));
    packet.write(&insId, sizeof(insId));
    ef->writeInitialState(packet);
    sendPersPacketFor(cl->ip, packet, ++cl->svAck); 
}

void Game::Server::notifyRemoveEffect(UInt64 uid, UInt16 insId){
    String msg = "failed to notify remove effect for client uid "+nite::toStr(uid)+": ";
    auto cl = getClient(uid);
    if(cl == NULL){
        nite::print(msg+"it doesn't exist");
        return;
    }
    nite::Packet packet(Game::PacketType::SV_REMOVE_EFFECT);
    packet.write(&cl->entityId, sizeof(cl->entityId));
    packet.write(&insId, sizeof(insId));
    sendPersPacketFor(cl->ip, packet, ++cl->svAck);
}

void Game::Server::notifyUpdateEffect(UInt64 uid, UInt16 insId){
    String msg = "failed to notify add effect for client uid "+nite::toStr(uid)+": ";
    auto cl = getClient(uid);
    if(cl == NULL){
        nite::print(msg+"it doesn't exist");
        return;
    }
    auto it = world.objects.find(cl->entityId);
    if(it == world.objects.end()){
        nite::print(msg+"doesn't have an active entity");
        return;
    }
    auto ent = static_cast<Game::EntityBase*>(world.objects[it->first].get());
    auto itef = ent->effectStat.effects.find(insId);
    if(itef == ent->effectStat.effects.end()){
        nite::print(msg+"doesn't have an effect insId "+nite::toStr(insId));
        return;
    }
    auto ef = ent->effectStat.effects[insId];
    nite::Packet packet(Game::PacketType::SV_UPDATE_EFFECT);
    packet.write(&cl->entityId, sizeof(cl->entityId));
    packet.write(&insId, sizeof(insId));
    ef->writeState(packet);
    sendPersPacketFor(cl->ip, packet, ++cl->svAck);
}

void Game::Server::notifyGameOver(UInt64 uid){
       String msg = "failed to notify game over for client uid "+nite::toStr(uid)+": ";
    auto cl = getClient(uid);
    if(cl == NULL){
        nite::print(msg+"it doesn't exist");
        return;
    }
    nite::Packet packet(Game::PacketType::SV_SET_GAME_OVER);
    sendPersPacketFor(cl->ip, packet, ++cl->svAck);
}


void Game::Server::notifyGameRestart(UInt64 uid){
       String msg = "failed to notify game over for client uid "+nite::toStr(uid)+": ";
    auto cl = getClient(uid);
    if(cl == NULL){
        nite::print(msg+"it doesn't exist");
        return;
    }
    nite::Packet packet(Game::PacketType::SV_SET_GAME_RESTART);
    sendPersPacketFor(cl->ip, packet, ++cl->svAck);
}

void Game::Server::notifyDeath(UInt64 uid){
    String msg = "failed to notify entity death for client uid "+nite::toStr(uid)+": ";
    // TODO(note?): We only want to notify clients who are able to see the entity in question
    auto cl = getClient(uid);
    if(cl == NULL){
        nite::print(msg+"it doesn't exist");
        return;
    }
    auto it = world.objects.find(cl->entityId);
    if(it == world.objects.end()){
        nite::print(msg+"doesn't have an active entity");
        return;
    }
    auto ent = static_cast<Game::EntityBase*>(world.objects[it->first].get());
    nite::Packet packet(Game::PacketType::SV_NOTIFY_ENTITY_DEATH);
    packet.write(&cl->entityId, sizeof(cl->entityId));
    ent->writeHealthStatState(packet);
    sendPersPacketFor(cl->ip, packet, ++cl->svAck);
}

void Game::Server::sendPlayerList(UInt64 uid){
    auto cl = getClient(uid);
    if(cl == NULL){
        nite::print("failed to send player list to client '"+nite::toStr(uid)+"': it's not connected");
        return;
    }
    UInt16 total = clients.size();
    nite::Packet info(Game::PacketType::SV_CLIENT_LIST);
    info.write(&total, sizeof(UInt16));
    for(auto cl : clients){
        info.write(&cl.second.clientId, sizeof(UInt64));
        info.write(&cl.second.ping, sizeof(UInt64));
        info.write(cl.second.nickname);
    }
    sendPersPacketFor(cl->ip, info, ++cl->svAck);
}

void Game::Server::sendSkillList(UInt64 uid, UInt16 entityId){
    auto itent = this->world.objects.find(entityId);
    auto cl = getClient(uid);
    if(cl == NULL){
        nite::print("[server] failed to skill list for entity: client id "+nite::toStr(uid)+" doesn't exist");
        return;
    }
    if(itent == this->world.objects.end()){
        nite::print("[server] failed to skill list for entity: ent id "+nite::toStr(entityId)+" doesn't exist");
        return;
    }
    auto ent = static_cast<Game::EntityBase*>(itent->second.get()); // we're gonna assume this is indeed an entity
    auto &sklst = ent->skillStat.skills;
    UInt8 skamnt = sklst.size();
    nite::Packet pck(Game::PacketType::SV_SET_ENTITY_SKILLS);
    pck.write(&ent->id, sizeof(UInt16));
    pck.write(&skamnt, sizeof(UInt8));
    for(auto sk : ent->skillStat.skills){
        pck.write(&sk.first, sizeof(UInt16));
        pck.write(&sk.second, sizeof(UInt8));
    }
    sendPersPacketFor(cl->ip, pck, ++cl->svAck);
}

Shared<Game::NetObject> Game::Server::spawn(Shared<Game::NetObject> obj){
    if(!init){
        return Shared<Game::NetObject>(NULL);
    }
    if(obj.get() == NULL){
        nite::print("cannot spawn undefined object");
        return Shared<Game::NetObject>(NULL);
    }    
    obj->sv = this; // we guarantee entities will have its sv ref as long as they're running on a server
    obj->net = this;
    obj->container = &world;
    auto id = this->world.add(obj);

    // solve position  
    Game::NetObject *who;
    int trial = 0;
    // while(obj->isCollidingWithSomething(&who) && obj->objType != ObjectType::Ghost){
    //     if(trial > 3 || who == NULL){
    //         break;
    //     }
    //     // hardcoded for now
    //     if(trial == 0){
    //         obj->setPosition(who->position.x + who->size.x, who->position.y);
    //     }else
    //     if(trial == 1){
    //         obj->setPosition(who->position.x - obj->size.x, who->position.y);
    //     }else
    //     if(trial == 2){
    //         obj->setPosition(who->position.x, who->position.y - obj->size.y);
    //     }else
    //     if(trial == 3){
    //         obj->setPosition(who->position.x, who->position.y + who->size.y);
    //     }
    //     ++trial;
    // }

    nite::Packet crt(Game::PacketType::SV_CREATE_OBJECT);
    crt.write(&id, sizeof(UInt16));
    crt.write(&obj->sigId, sizeof(UInt16));
    crt.write(&obj->position.x, sizeof(float));
    crt.write(&obj->position.y, sizeof(float));
    obj->writeInitialState(crt);
    sendPersPacketForMany(getAllClientsIps(), crt, getAllClientsAcks());
    return obj;
}

bool Game::Server::destroy(UInt16 id){
    auto obj = world.get(id);
    if(obj == NULL){
        nite::print("[server] failed to destroy object id '"+nite::toStr(id)+"': it doesn't exist");
        return false;
    }
    nite::Packet des(Game::PacketType::SV_DESTROY_OBJECT);
    des.write(&id, sizeof(UInt32));
    obj->destroy();
    obj->sv = NULL;
    obj->container = NULL; 
    sendPersPacketForMany(getAllClientsIps(), des, getAllClientsAcks());
    return true;
}

Shared<Game::NetObject>  Game::Server::createMob(UInt16 sig, UInt32 lv, float x, float y){
    auto obj = Game::createNetObject(world.generateId(), sig, x, y);
    if(obj.get() == NULL){
        nite::print("[server] FATAL ERROR: failed to create mob object: signature id '"+Game::ObjectSig::name(sig)+"' does not exist?");
        close();
        return obj;
    }    
    Game::EntityBase *ent = static_cast<Game::EntityBase*>(obj.get());
    ent->setupStat(lv);
    spawn(obj);
    ent->loadAnim();
    ent->printInfo();
    nite::print("[server] created mob entity with id "+nite::toStr(obj->id));
    return obj;
}

Shared<Game::NetObject> Game::Server::createPlayer(UInt64 uid, UInt32 lv, float x, float y){
    auto client = clients.find(uid);
    if(client == clients.end()){
        nite::print("[server] failed to create player for unexisting client id '"+nite::toStr(uid)+"'");
        return Shared<Game::NetObject>(NULL);
    }

    auto obj = Game::createNetObject(world.generateId(), Game::ObjectSig::Player, x, y);
    if(obj.get() == NULL){
        nite::print("[server] FATAL ERROR: failed to create player object: signature id '"+Game::ObjectSig::name(Game::ObjectSig::Player)+"' does not exist?");
        close();
        return obj;
    }
    Game::EntityBase *player = static_cast<Game::EntityBase*>(obj.get());
    player->setupStat(lv);
    spawn(obj);
    client->second.entityId = obj->id;
    player->loadAnim();
    players[uid] = obj->id;

    nite::print("[server] created player entity with id "+nite::toStr(obj->id)+" | for client id "+nite::toStr(uid)+"("+client->second.nickname+")");

    // testing stats
    // static_cast<Game::EntityBase*>(obj.get())->addBaseStat(BaseStatType::Agility, 100);
    // testing effects
    auto effect = getEffect(Game::EffectList::EF_HEAL);
    auto efHeal = static_cast<Game::Effects::EffHeal*>(effect.get());
    efHeal->setup(100, 5 * 1000);
    this->addEffect(obj->id, effect);
    player->printInfo(); // for debugging
    auto bow = Game::getItem(ItemList::W_BOW, 1);
    auto arrows = Game::getItem(ItemList::AM_ARROW, 200);
    auto sword = Game::getItem(ItemList::W_SWORD, 1);
    player->invStat.add(bow);
    player->invStat.add(arrows);
    player->invStat.add(sword);
    // player->invStat.equip(sword);
    player->invStat.equip(bow);
    player->invStat.equip(arrows);
    return obj;
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
    auto client = getClient(uid);
    if(client != NULL){
        nite::print("failed to kill player whose id is "+nite::toStr(uid)+": it doesn't exist");
        return false;
    }
    String generic = "player id "+nite::toStr(client->entityId)+"("+client->nickname+")";
    auto it = world.objects.find(client->entityId);
    if(it == world.objects.end()){
        nite::print("failed to kill "+generic+": it doesn't exist");
        return false;
    }
    auto entity = static_cast<Game::EntityBase*>(it->second.get());
    if(entity->healthStat.dead){
        nite::print(generic+" is already dead");
        return false;
    }
    entity->kill();
    nite::print(generic+" was killed");
    return true;    
}
