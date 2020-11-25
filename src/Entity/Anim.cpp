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
    if(node.has("useLooseLimb") && node.get("useLooseLimb").toBool(false)){
        useLooseLimbs = true;
        auto innode = node.get("looseLimb");
        auto framesize = nite::Vec2(innode.get("framesize").get("w").toInt(32), innode.get("framesize").get("h").toInt(32));
        limbSize = framesize;
        // types
        for(auto &limbnode : innode.get("types")){
            Game::AnimTypeLimb limb;
            limb.name = limbnode.first;
            limb.inTexSize = framesize;
            limb.inTexCoors = nite::Vec2(limbnode.second.get("x").toFloat(), limbnode.second.get("y").toInt());
            limbTypes[limb.name] = limb;
        }
        // limbs
        for(auto &limbnode : innode.get("limbs")){
            Game::AnimLimb limb;
            limb.name = limbnode.first;
            limb.type = "open_hand";
            limb.depth = limbnode.second.get("depth").toInt();
            limbs[limb.name] = limb;
        }        
    }    
    auto textureSize = nite::Vec2(node.get("textureSize").get("w").toInt(0), node.get("textureSize").get("h").toInt(0));
    for(auto &fnode : node.get("frames")){
        Game:: AnimFrame frame;
        frame.n = fnode.second.get("n").toInt(0);
        frame.type = AnimType::getType(fnode.first);
        frame.x = fnode.second.get("x").toInt(0);
        frame.y = fnode.second.get("y").toInt(0);
        frame.spd = fnode.second.get("spd").toInt(0);
        frame.keyframe = fnode.second.get("kf").toInt(0);
        if(useLooseLimbs && fnode.second.has("limbs")){
            auto limbs = fnode.second.get("limbs");
            for(auto &limb : limbs){
                Vector<AnimLimbFrame> frames;
                String name = limb.first;
                for(auto &frame : limb.second){
                    AnimLimbFrame nframe;
                    nframe.limb = name;
                    nframe.an = frame.second.get("angle").toFloat();
                    nframe.p.x = frame.second.get("pos").get("x").toFloat();
                    nframe.p.y = frame.second.get("pos").get("y").toFloat();
                    nframe.xflip = frame.second.get("xflip").toBool(false);
                    nframe.yflip = frame.second.get("yflip").toBool(false);
                    frames.push_back(nframe);
                }
                frame.limbs[name] = frames;
            }
        }        
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
    useLooseLimbs = false;
}

void Game::Anim::setState(UInt8 anims[AnimPart::total], UInt8 sframes[AnimPart::total]){
    for(int i = 0; i < AnimPart::total; ++i){
        auto it = this->frames.find(anims[i]);
        if(it == this->frames.end()){
            lastSFrame[i] = -1;
            continue;
        }        
        lastAnim[i] = it->second;
        lastSFrame[i] = sframes[i] % lastAnim[i].n;
    }
    rerender();
}

void Game::Anim::rerender(){
    batch.init(frameSize.x, frameSize.y);
    batch.begin();
    nite::setColor(1.0f, 1.0f, 1.0f, 1.0f);
    for(int i = 0; i < AnimPart::total; ++i){
        if(lastSFrame[i] == -1){
            continue;
        }
        auto &frame = lastAnim[i];
        int nf = lastSFrame[i];
        if(frame.limbs.size() > 0){
            for(auto &limb : limbs){
                auto &frameLimb = frame.limbs[limb.first][nf];
                limb.second.npos = frameLimb.p;
                limb.second.nangle = frameLimb.an;
                limb.second.xflip = frameLimb.xflip;
                limb.second.yflip = frameLimb.yflip;
            }
        }
        nite::setDepth(0);
        anim.setFrame(frame.id, nf);
        auto ref = anim.draw(frame.id, 0.0f, 0.0f, frameSize.x, frameSize.y, 0.0f, 0.0f, 0.0f);
    }    
    if(useLooseLimbs){
        for(auto &limb : limbs){
            auto &type = limbTypes[limb.second.type];
            nite::setDepth(limb.second.depth);
            anim.texture.setRegion(type.inTexCoors, type.inTexSize);
            anim.texture.draw(limb.second.pos.x, limb.second.pos.y, limbSize.x * (limb.second.xflip ? -1.0f: 1.0f), limbSize.y * (limb.second.yflip ? -1.0f: 1.0f), 0.5f, 0.5f, limb.second.angle);
        }
    }
    batch.end();
    batch.flush();
}

void Game::Anim::update(){
    bool dorerender = false;
    for(auto &limb : limbs){
        dorerender = limb.second.pos.lerpDiscrete(limb.second.npos, 0.25f) || nite::lerpDiscrete(limb.second.angle, limb.second.nangle, 0.25f);
    }
    if(dorerender){
        rerender();
    }
}

Game::AnimFrame *Game::Anim::getAnim(UInt8 anim){
    auto it = this->frames.find(anim);
    if(it != this->frames.end()){
        return &it->second;
    }
    return NULL;
}