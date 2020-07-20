#include <algorithm>

#include "../Engine/Tools/Tools.hpp"
#include "../Engine/Console.hpp"
#include "../Engine/UI/UI.hpp"

#include "../Game.hpp"
#include "Server.hpp"
#include "Input.hpp"
#include "Object.hpp"
#include "../Entity/Base.hpp"

static Shared<nite::BaseUIComponent> debug = Shared<nite::BaseUIComponent>(NULL);
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
    dmg.owner = Shared<Game::EntityBase>(NULL);
    dmg.receiver = obj;
    dmg.truedmg = false;
    dmg.weap = Shared<Game::EquipItem>(NULL);
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
    sv.dropClient(client->clientId, reason);
    return nite::Console::Result();
}
static auto cfKickIns = nite::Console::CreateFunction("kick", &cfKick, true, true);


Game::Server::Server(){
    this->init = false;
    setState(Game::ServerState::Off);
}

void Game::Server::setState(unsigned state){
    this->state = state;
    lastState = nite::getTicks();
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
    ft.listen(nite::NetworkDefaultFileTransferPort);
    nite::print("[server] started server '"+name+"' | max clients "+nite::toStr(this->maxClients)+" | listening at "+nite::toStr(port));
    nite::print("[server] waiting for clients...");
    this->init = true;
    setState(Game::ServerState::Idle);

    if(debugging){
        debug = nite::UI::build("./data/ui/debug_window.json");
        auto json = Jzon::object();
        json.add("type", "text");
        json.add("fontColor", "#000000");
        json.add("fontSize", "16");
        json.add("text", "debug");
        debug->add(nite::UI::build(json));
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

Game::SvClient *Game::Server::getClientByIp(nite::IP_Port &ip){
    for(auto _cl : clients){
        if(_cl.second.cl.isSame(ip)){
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

void Game::Server::dropClient(UInt64 uid, String reason){
    auto cl = this->getClient(uid);
    if(cl == NULL){
        nite::print("[server] cannot drop client "+nite::toStr(uid)+": it's not connected");
        return;
    }
    if(cl->entityId != 0){
        destroy(cl->entityId); // destroy player's entity
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

void Game::Server::sendRemoteCmdMsg(UInt64 uid, const String &msg, const nite::Color &color){
    auto cl = this->getClient(uid);
    if(cl == NULL){
        nite::print("[server] cannot send remote cmd msg to client "+nite::toStr(uid)+": it's not connected");
        return;
    }
    nite::Packet msgPacket(++cl->svOrder);
    msgPacket.setHeader(Game::PacketType::SV_REMOTE_CMD_MSG);
    msgPacket.write(msg);
    msgPacket.write(&color.r, sizeof(float));
    msgPacket.write(&color.g, sizeof(float));
    msgPacket.write(&color.b, sizeof(float));
    persSend(cl->cl, msgPacket, 500, -1);    
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
    setState(Game::ServerState::ShuttingDown);
    broadcast("server is closing down");
    nite::print("[server] closing server down, dropping clients...");
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
    physicsUpdate = nite::getTicks();
    sock.close();
    world.clearWallMasks();
    ft.clear();
    deliveries.clear();
    clients.clear();
    players.clear();
    maps.clear();
    tilesets.clear();
    world.clear();
    setState(Game::ServerState::Off);
    if(debugging && debug.get() != NULL){
        static_cast<nite::WindowUI*>(debug.get())->close();
    }    
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
        UInt64 netId = sender.address+sender.port+sock.sock;
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
                    String msg = "server is full";
                    reject.write(msg);
                    nite::print("[server] rejected client "+nite::toStr(netId)+": "+msg);
                    sock.send(sender, reject); 
                    break;
                }
                // TODO: add option to allow campaigns to let players join midgame
                if(state == Game::ServerState::InGame){
                    nite::Packet reject((UInt32)0);
                    reject.setHeader(Game::PacketType::SV_CONNECT_REJECT);
                    String msg = "game already started";
                    reject.write(msg);
                    nite::print("[server] rejected client "+nite::toStr(netId)+": "+msg);
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
                    nite::SmallPacket netIdPack;
                    netIdPack.write(&netId, sizeof(UInt64));
                    nite::print("[server] accepted clientId "+nite::toStr(netId)+" | nickname '"+nick+"'");
                    if(clients.size() > 0 && state == Game::ServerState::Idle){
                        nite::print("[server] awaiting for client to load...");
                        setState(Game::ServerState::Waiting);
                    }
                    persSend(clients[netId].cl, accepted);
                    bindOnAckFor(Game::PacketType::SV_CONNECT_ACCEPT, [&](nite::SmallPacket &payload, nite::IP_Port &cl){
                        auto client = getClientByIp(cl);
                        if(client == NULL){
                            nite::print("[server] SV_CONNECT_REQUEST -> bindOnAckFor::SV_CONNECT_ACCEPT: fatal failure: client doesn't exist anymore?");
                            return;
                        }
                        nite::Packet await(++client->svOrder);
                        await.setHeader(Game::PacketType::SV_AWAIT_CLIENT_LOAD);
                        await.write(maps[currentMap]->hash);
                        persSend(cl, await, 1000, -1);
                        bindOnAckFor(Game::PacketType::SV_AWAIT_CLIENT_LOAD, [&](nite::SmallPacket &payload, nite::IP_Port &cl){
                            auto client = getClientByIp(cl);
                            nite::Packet notify;
                            notify.setHeader(Game::PacketType::SV_CLIENT_JOIN);
                            notify.write(&client->clientId, sizeof(client->clientId));
                            notify.write(client->nickname);
                            persSendAll(notify, 1000, -1);     
                            sendPlayerList(client->clientId);                            
                        });
                    });
                    
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
                if(!client || !isLast){
                    break;
                }
                auto ent = getEntity(client->entityId);
                if(ent != NULL){
                    UInt16 compat;
                    handler.read(&compat, sizeof(compat));
                    ent->input.loadCompat(compat);
                }
            } break;       
            /*
                SV_RCON
            */
            case Game::PacketType::SV_RCON: {
                if(!client){
                    break;
                }
                sendAck(client->cl, handler.getOrder(), ++client->lastSentOrder);
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
                if(!client){
                    break;
                }
                sendAck(client->cl, handler.getOrder(), ++client->lastSentOrder);
                String cmd;
                handler.read(cmd);
                auto result = nite::Console::interpret(cmd, false, true, client->role == Game::SvClientRole::Admin);
                sendRemoteCmdMsg(client->clientId, result.msg, result.color);
            } break;  
            /*
                SV_CLIENT_LOAD_READY
            */             
            case Game::PacketType::SV_CLIENT_LOAD_READY: {
                if(!client){
                    break;
                }
                sendAck(client->cl, handler.getOrder(), ++client->lastSentOrder);
                client->ready = true;
            } break;             
            /*
                SV_ENTITY_USE_SKILL_ITEM
            */             
            case Game::PacketType::SV_ENTITY_USE_SKILL_ITEM: {
                if(!client){
                    break;
                }
                sendAck(client->cl, handler.getOrder(), ++client->lastSentOrder);
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
    }

    // refresh player info
    if(nite::getTicks()-lastPlayerInfoSent > 1000){
        lastPlayerInfoSent = nite::getTicks();
        for(auto cl : clients){
            sendPlayerList(cl.first);
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
    for(auto &cl : clients){
        auto &client = cl.second;
        if(nite::getTicks()-client.lastPing > 1000){
            client.lastPing = nite::getTicks();
            nite::Packet ping(++client.lastSentOrder);
            ping.setHeader(Game::PacketType::SV_PING);
            UInt64 ticks = nite::getTicks();
            ping.write(&ticks, sizeof(ticks));
            sock.send(client.cl, ping);
        }
    }

    // update physics
    if(nite::getTicks()-physicsUpdate > 0){
        auto &queue = world.updateQueue;
        if(queue.size() > 0){      
            UInt16 amnt = queue.size();
            nite::Packet phys;
            phys.setHeader(Game::PacketType::SV_UPDATE_PHYSICS_OBJECT);
            phys.write(&amnt, sizeof(UInt16));
            // TODO: scope it for visible areas only            
            for(auto &it : queue){
                auto &obj = world.objects[it.first];
                phys.write(&obj->id, sizeof(UInt16));
                phys.write(&obj->position.x, sizeof(float));
                phys.write(&obj->position.y, sizeof(float));
                phys.write(&obj->speed.x, sizeof(float));
                phys.write(&obj->speed.y, sizeof(float));
            }
            sendAll(phys);
            queue.clear();
        }
        physicsUpdate = nite::getTicks();
    }  

    //TODO: update animations

    game();
    updateDeliveries();
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


void Game::Server::sendAll(nite::Packet &packet){
    if(!init){
        return;
    }    
    for(auto &cl : clients){
        nite::Packet cpy = packet;
        cpy.setOrder(++cl.second.lastSentOrder);
        sock.send(cl.second.cl, cpy);
    }
}

void Game::Server::persSendAll(nite::Packet &packet, UInt64 timeout, int retries){
    if(!init){
        return;
    }    
    for(auto &cl : clients){
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

    // update entity specifics
    for(auto &obj : world.objects){
        if(obj.second->objType == ObjectType::Entity){
            auto ent = static_cast<Game::EntityBase*>(obj.second.get());
            auto &in = ent->input;
            bool isSpace = in.isKeyPress(Game::Key::SPACE);
            if(in.isKeyPress(Game::Key::UP) && in.isKeyPress(Game::Key::RIGHT)){
                ent->entityMove(nite::Vec2(1.0f, -1.0f), isSpace);
            }else
            if(in.isKeyPress(Game::Key::DOWN) && in.isKeyPress(Game::Key::RIGHT)){
                ent->entityMove(nite::Vec2(1.0f, 1.0f), isSpace);
            }else
            if(in.isKeyPress(Game::Key::UP) && in.isKeyPress(Game::Key::LEFT)){
                ent->entityMove(nite::Vec2(-1.0f, -1.0f), isSpace);
            }else		
            if(in.isKeyPress(Game::Key::DOWN) && in.isKeyPress(Game::Key::LEFT)){
                ent->entityMove(nite::Vec2(-1.0f, 1.0f), isSpace);
            }else				
            if(in.isKeyPress(Game::Key::UP)){
                ent->entityMove(nite::Vec2(0.0f, -1.0f), isSpace);
            }else
            if(in.isKeyPress(Game::Key::RIGHT)){
                ent->entityMove(nite::Vec2(1.0f, 0.0f), isSpace);
            }else
            if(in.isKeyPress(Game::Key::DOWN)){
                ent->entityMove(nite::Vec2(0.0f, 1.0f), isSpace);
            }else
            if(in.isKeyPress(Game::Key::LEFT)){
                ent->entityMove(nite::Vec2(-1.0f, 0.0f), isSpace);
            }
            ent->effectStat.update();
            ent->entityStep();
        }
    }
    
    // update local physics
    world.update();
}

void Game::Server::createPlayersOnStart(UInt16 initialHeader){
    nite::Packet simProps;
    simProps.setHeader(Game::PacketType::SV_UPDATE_WORLD_SIMULATION_PROPS);
    simProps.write(&world.timescale, sizeof(world.timescale));
    simProps.write(&world.tickrate, sizeof(world.tickrate));
    persSendAll(simProps, 750, -1);
    auto me = this;
    // and now some chained-deliveries craziness... (apologies in advance)

    // [0] bind on the initialHeader before we start creating players and asigning
    me->bindOnAckFor(initialHeader, [me](nite::SmallPacket &pck, nite::IP_Port &ip){
        auto cl = me->getClientByIp(ip);
        if(cl == NULL){
            nite::print("[server] failed to notify clients their respective entity. ip was not found in the list");
            return;
        }

        float startx = me->maps[me->currentMap]->startCell.x + nite::randomInt(-50, 50);
        float starty = me->maps[me->currentMap]->startCell.y + nite::randomInt(-50, 50);    

        me->createPlayer(cl->clientId, 1, startx, starty);
        // [1] notify clients of their respective owners
        me->bindOnAckFor(Game::PacketType::SV_CREATE_OBJECT, [me](nite::SmallPacket &pck, nite::IP_Port &ip){   
            auto cl = me->getClientByIp(ip);
            if(cl == NULL){
                nite::print("[server] failed to notify clients their respective entity. ip was not found in the list");
                return;
            }
            nite::Packet noti(++cl->svOrder);
            noti.setHeader(Game::PacketType::SV_NOTI_ENTITY_OWNER);
            noti.write(&cl->entityId, sizeof(UInt16));
            me->persSend(cl->cl, noti);         
            // [2] send respective clients their default skill list
            me->bindOnAckFor(Game::PacketType::SV_NOTI_ENTITY_OWNER, [me](nite::SmallPacket &pck, nite::IP_Port &ip){   
                auto cl = me->getClientByIp(ip);
                if(cl == NULL){
                    nite::print("[server] failed to send skill list. ip was not found in the list");
                    return;
                }
                me->sendSkillList(cl->clientId, cl->entityId);
                // [3] send respective clients their default keybinds
                me->bindOnAckFor(Game::PacketType::SV_SET_ENTITY_SKILLS, [me](nite::SmallPacket &pck, nite::IP_Port &ip){  
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
                    nite::Packet packet(++cl->svOrder);
                    packet.setHeader(Game::PacketType::SV_SET_ENTITY_ACTIONABLES);
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
                    writeSkill(Game::SkillList::BA_DODGE, 1);
                    writeSkill(Game::SkillList::BA_FIRST_AID, 2);
                    me->persSend(cl->cl, packet, 750, -1);
                });                    
            });             
        });             
    }); 


    float startx = me->maps[me->currentMap]->startCell.x + nite::randomInt(-50, 50);
    float starty = me->maps[me->currentMap]->startCell.y + nite::randomInt(-50, 50);   

    auto objMob = createMob(Game::ObjectSig::MobHumanoid, 10, startx, starty); 
    auto mob = static_cast<Game::EntityBase*>(objMob.get());
    mob->aidriver.add(Shared<Game::AI::DumbassBehavior>(new Game::AI::DumbassBehavior()));
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
    world.clear();
    nite::Packet restart;
    restart.setHeader(Game::PacketType::SV_SET_GAME_RESTART);
    persSendAll(restart, 1000, -1);    
    createPlayersOnStart(Game::PacketType::SV_SET_GAME_RESTART); 
}

void Game::Server::start(){
    setState(Game::ServerState::InGame);
    nite::Packet start;
    start.setHeader(Game::PacketType::SV_SET_GAME_START);
    persSendAll(start, 1000, -1);  
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
    Game::RING::TileSource src("data/tileset/dungeon.json");
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
    setCurrentMap(0);
    listen(name, maxClients, nite::NetworkDefaultPort);
}

void Game::Server::setCurrentMap(unsigned cm){
    if(cm >= this->maps.size()){
        return;
    }
    world.clearWallMasks();
    this->currentMap = cm;
    auto &m = this->maps[this->currentMap]; 
    nite::Vec2 ws = m->size * m->tileSize;
    this->world.setSize(ws.x, ws.y, 128, 128);    
    for(int i = 0; i < m->masks.size(); ++i){
        auto &mask = m->masks[i];
        auto obj = new Game::NetObject();
        obj->position = mask.position;
        obj->size = mask.size;
        obj->solid = true;
        obj->unmovable = true;
        this->world.addWallMask(obj);
        localMasks.push_back(obj);
    }
    nite::print("current cmasks: "+nite::toStr(this->world.objects.size()));
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
    nite::Packet packet(++cl->svOrder);
    packet.setHeader(Game::PacketType::SV_ADD_ITEM);
    packet.write(&cl->entityId, sizeof(cl->entityId));
    packet.write(&itemId, sizeof(itemId));
    packet.write(&slotId, sizeof(slotId));
    packet.write(&qty, sizeof(qty));
    persSend(cl->cl, packet, 750, -1);    
}

void Game::Server::notifyRemoveItem(UInt64 uid, UInt16 itemId, UInt16 slotId, UInt16 qty){
    auto cl = getClient(uid);
    if(cl == NULL){
        return;
    }
    nite::Packet packet(++cl->svOrder);
    packet.setHeader(Game::PacketType::SV_REMOVE_ITEM);
    packet.write(&cl->entityId, sizeof(cl->entityId));
    packet.write(&itemId, sizeof(itemId));
    packet.write(&slotId, sizeof(slotId));
    packet.write(&qty, sizeof(qty));
    persSend(cl->cl, packet, 750, -1);  
}

void Game::Server::notifyAddSkill(UInt64 uid, UInt16 skillId, UInt8 lv){
    auto cl = getClient(uid);
    if(cl == NULL){
        return;
    }
    nite::Packet packet(++cl->svOrder);
    packet.setHeader(Game::PacketType::SV_ADD_ENTITY_SKILL);
    packet.write(&cl->entityId, sizeof(cl->entityId));
    packet.write(&skillId, sizeof(skillId));
    packet.write(&lv, sizeof(lv));
    persSend(cl->cl, packet, 750, -1);
}

void Game::Server::notifyRemoveSkill(UInt64 uid, UInt16 skillId){
    auto cl = getClient(uid);
    nite::Packet packet(++cl->svOrder);
    packet.setHeader(Game::PacketType::SV_REMOVE_ENTITY_SKILL);
    packet.write(&cl->entityId, sizeof(cl->entityId));
    packet.write(&skillId, sizeof(skillId));
    persSend(cl->cl, packet, 750, -1);
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
    nite::Packet packet(++cl->svOrder);
    packet.setHeader(Game::PacketType::SV_ADD_EFFECT);
    packet.write(&cl->entityId, sizeof(cl->entityId));
    packet.write(&type, sizeof(type));
    packet.write(&insId, sizeof(insId));
    ef->writeInitialState(packet);
    persSend(cl->cl, packet, 750, -1);    
}

void Game::Server::notifyRemoveEffect(UInt64 uid, UInt16 insId){
    String msg = "failed to notify remove effect for client uid "+nite::toStr(uid)+": ";
    auto cl = getClient(uid);
    if(cl == NULL){
        nite::print(msg+"it doesn't exist");
        return;
    }
    nite::Packet packet(++cl->svOrder);
    packet.setHeader(Game::PacketType::SV_REMOVE_EFFECT);
    packet.write(&cl->entityId, sizeof(cl->entityId));
    packet.write(&insId, sizeof(insId));
    persSend(cl->cl, packet, 750, -1);  
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
    nite::Packet packet(++cl->svOrder);
    packet.setHeader(Game::PacketType::SV_UPDATE_EFFECT);
    packet.write(&cl->entityId, sizeof(cl->entityId));
    packet.write(&insId, sizeof(insId));
    ef->writeState(packet);
    persSend(cl->cl, packet, 750, -1);  
}

void Game::Server::notifyGameOver(UInt64 uid){
       String msg = "failed to notify game over for client uid "+nite::toStr(uid)+": ";
    auto cl = getClient(uid);
    if(cl == NULL){
        nite::print(msg+"it doesn't exist");
        return;
    }
    nite::Packet packet(++cl->svOrder);
    packet.setHeader(Game::PacketType::SV_SET_GAME_OVER);
    persSend(cl->cl, packet, 750, -1);   
}


void Game::Server::notifyGameRestart(UInt64 uid){
       String msg = "failed to notify game over for client uid "+nite::toStr(uid)+": ";
    auto cl = getClient(uid);
    if(cl == NULL){
        nite::print(msg+"it doesn't exist");
        return;
    }
    nite::Packet packet(++cl->svOrder);
    packet.setHeader(Game::PacketType::SV_SET_GAME_RESTART);
    persSend(cl->cl, packet, 750, -1);   
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
    nite::Packet packet(++cl->svOrder);
    packet.setHeader(Game::PacketType::SV_NOTIFY_ENTITY_DEATH);
    packet.write(&cl->entityId, sizeof(cl->entityId));
    ent->writeHealthStatState(packet);
    persSend(cl->cl, packet, 750, -1);
}

void Game::Server::sendPlayerList(UInt64 uid){
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
    nite::Packet pck(++cl->svOrder);
    pck.setHeader(Game::PacketType::SV_SET_ENTITY_SKILLS);
    pck.write(&ent->id, sizeof(UInt16));
    pck.write(&skamnt, sizeof(UInt8));
    for(auto sk : ent->skillStat.skills){
        pck.write(&sk.first, sizeof(UInt16));
        pck.write(&sk.second, sizeof(UInt8));
    }
    persSend(cl->cl, pck, 750, -1);
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
    while(obj->isCollidingWithSomething(&who)){
        if(trial > 3 || who == NULL){
            break;
        }
        // hardcoded for now
        if(trial == 0){
            obj->setPosition(who->position.x + who->size.x, who->position.y);
        }else
        if(trial == 1){
            obj->setPosition(who->position.x - obj->size.x, who->position.y);
        }else
        if(trial == 2){
            obj->setPosition(who->position.x, who->position.y - obj->size.y);
        }else
        if(trial == 3){
            obj->setPosition(who->position.x, who->position.y + who->size.y);
        }
        ++trial;
    }

    nite::Packet crt;
    crt.setHeader(Game::PacketType::SV_CREATE_OBJECT);
    crt.write(&id, sizeof(UInt16));
    crt.write(&obj->sigId, sizeof(UInt16));
    crt.write(&obj->position.x, sizeof(float));
    crt.write(&obj->position.y, sizeof(float));
    obj->writeInitialState(crt);
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
    obj->sv = NULL;
    obj->container = NULL;
    persSendAll(des, 1000, -1);    
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