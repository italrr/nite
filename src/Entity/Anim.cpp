#include "Anim.hpp"
#include "../Engine/Tools/Tools.hpp"
#include "../Engine/Indexer.hpp"

bool Game::Anim::load(const String &path){
    String errmsg = "failed to load anim file '"+path+"': ";
    if(!nite::fileExists(path)){
        nite::print(errmsg+"it doesn't exist");
        return false;
    }
    Jzon::Parser parser;
    Jzon::Node node = parser.parseFile(path);
    if(!node.isValid()){
        nite::print(errmsg+parser.getError());
        return false;
    }
    auto indexer = nite::getIndexer();
    frameSize.set(node.get("frameSize").get("w").toInt(128), node.get("frameSize").get("h").toInt(192));
    String hash = node.get("source").toString("");
    auto ifile = indexer->get(hash);
    this->transparency = nite::Color(node.get("transparency").toString("#ffffff"));
    if(ifile == NULL){
        nite::print(errmsg+"source file '"+hash+"' was not found");
        return false;
    }
    if(ifile != NULL && !ifile->isIt("spritesheet")){
        nite::print(errmsg+"source file is not a spritesheet");
        return false;
    }
    this->bodyDepthOffset = node.get("bodyDepthOffset").toFloat(0.0f);
    this->source = *ifile;
    if(!node.has("frames") || node.has("frames") && !node.get("frames").isObject()){
        nite::print(errmsg+"'frames' was not found or it's not an object");
        return false;
    }
    for(auto &fnode : node.get("frames")){
        Game:: AnimFrame frame;
        frame.n = fnode.second.get("n").toInt(0);
        frame.type = AnimType::getType(fnode.first);
        frame.x = fnode.second.get("x").toInt(0);
        frame.y = fnode.second.get("y").toInt(0);
        frame.spd = fnode.second.get("spd").toInt(0);
        frame.keyframe = fnode.second.get("kf").toInt(0);
        frame.id = this->anim.add(frame.x * frameSize.x, frame.y * frameSize.y, frameSize.x, frameSize.y, frame.n, frame.spd, false, false);
        this->anim.setManualClicking(frame.id, true);
        this->frames[frame.type] = frame;
    }
    for(auto &hnode : node.get("bodyHitbox")){
        nite::Hitbox hitbox;
        hitbox.name = hnode.second.get("name").toString("none");
        hitbox.position.x = hnode.second.get("x").toInt(0);
        hitbox.position.y = hnode.second.get("y").toInt(0);
        hitbox.size.x =  hnode.second.get("w").toInt(frameSize.x);
        hitbox.size.y =  hnode.second.get("h").toInt(frameSize.y);
        hitboxes.push_back(hitbox);
    }    
    if(node.has("meleeNoWeapHitbox")){
        auto obj = node.get("meleeNoWeapHitbox");
        meleeNoWeapHb.position.x = obj.get("x").toInt(0);
        meleeNoWeapHb.position.y = obj.get("y").toInt(0);
        meleeNoWeapHb.size.x = obj.get("w").toInt(0);
        meleeNoWeapHb.size.y = obj.get("h").toInt(0);        
        meleeNoWeapHb.name = "meleeNoWeap";
    }
    if(node.has("maskSize")){
        auto obj = node.get("maskSize");
        maskSize.x = obj.get("w").toInt(frameSize.x);
        maskSize.y = obj.get("h").toInt(frameSize.y);
    }    
    nite::print("loaded anim '"+path+"': "+nite::toStr(this->frames.size())+" animation(s)");
    return true;
}


Game::Anim::Anim(){
    for(int i = 0; i < AnimPart::total; ++i){
        this->parts[i] = AnimType::UNDEFINED;
    }
}

void Game::Anim::setState(UInt8 anims[AnimPart::total], UInt8 sframes[AnimPart::total]){
    batch.init(frameSize.x, frameSize.y);
    batch.begin();
    nite::setColor(1.0f, 1.0f, 1.0f, 1.0f);
    for(int i = 0; i < AnimPart::total; ++i){
        auto it = this->frames.find(anims[i]);
        if(it == this->frames.end()){
            continue;
        }
        auto &frame = it->second;
        anim.setFrame(frame.id, sframes[i] % frame.n);
        auto ref = anim.draw(frame.id, 0.0f, 0.0f, frameSize.x, frameSize.y, 0.0f, 0.0f, 0.0f);
    }    
    batch.end();
    batch.flush();
}

Game::AnimFrame *Game::Anim::getAnim(UInt8 anim){
    auto it = this->frames.find(anim);
    if(it != this->frames.end()){
        return &it->second;
    }
    return NULL;
}