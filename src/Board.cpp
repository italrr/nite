#include "Engine/Shader.hpp"
#include "Engine/Input.hpp"
#include "Board.hpp"
#include "Network.hpp"
#include "State.hpp"

void Board::init(const nite::Color &bg, int width, int height, const Shared<GameType> &gType, bool headless){
    this->bg = bg;
    this->gType = gType;
    this->width = width;
    this->height = height;
    this->unitSize.width = width / gType->width;
    this->unitSize.height = height / gType->height;    


    // generate board spaces (for client side)
    if(!headless){
        this->empty.load("data/texture/empty.png");
        this->shader.load("data/shaders/space_background_f.glsl", "data/shaders/space_background_v.glsl");
        rerender();
    }

    // generate space positions
    auto generateSpace = [&](const Space &space, const SpaceLayout &layout){
        const float width = this->unitSize.width * layout.width;
        const float height = this->unitSize.height * layout.height;        
        float rotw = width * nite::cos(nite::toRadians(layout.orientation)) - height * nite::sin(nite::toRadians(layout.orientation));
        float roth = width * nite::sin(nite::toRadians(layout.orientation)) + height * nite::cos(nite::toRadians(layout.orientation));
        float x = (this->unitSize.width * layout.x) + nite::abs(rotw)*0.5;
        float y = (this->unitSize.height * layout.y) + nite::abs(roth)*0.5;
        return nite::Vec2(x, y);
    };
    for(auto &it : gType->layout){
        auto &space = gType->spaces[it.first];
        this->spaces[space.id] = generateSpace(space, it.second);
    }   

}

void Board::rerender(){
    float ratio = (float)(width+height) / (1080.0f + 1080.0f);
    float fontsize = 11.0f * ratio;
    if(!this->font.isLoaded() || this->font.getFontSize() != fontsize){
        this->font.load("data/font/BebasNeueRegular-X34j2.otf", 12.0f * ratio, 0.0f);    
    }
    batch.init(width, height);
    auto generateSpace = [&](const Space &space, const SpaceLayout &layout, nite::Batch &batch){
        // generate space
        nite::Batch spaceb;
        const float width = this->unitSize.width * layout.width;
        const float height = this->unitSize.height * layout.height;
        const nite::Color titleColor = space.type == SPACE_TYPE::PROPERTY ? space.color : nite::Color();
        const bool isProperty = space.type == SPACE_TYPE::PROPERTY || space.type == SPACE_TYPE::UTILITY || space.type == SPACE_TYPE::RAILROAD;        
        spaceb.init(width, height);
        spaceb.begin();
            nite::setColor(0.0f, 0.0f, 0.0f, 1.0f);
            auto body = empty.draw(0, 0, width, height, 0.0f, 0.0f, 0.0f);
            nite::Uniform uniform;
            uniform.add("size", nite::Vec2(width, height));
            uniform.add("border", 1.0f);
            uniform.add("baseColor", bg);
            uniform.add("borderColor", nite::Color(0.0f, 0.0f, 0.0f, 1.0f));
            uniform.add("useTitle", space.type == SPACE_TYPE::PROPERTY);
            uniform.add("titleHeight", height*(1.0f/6.0f));
            uniform.add("titleColor", titleColor);
            body->apply(shader, uniform);
            nite::setColor(0.0f, 0.0f, 0.0f, 1.0f);
            auto robj = this->font.draw(nite::toUpper(space.name), width * 0.5f, height * 0.5f, 0.5f, 0.5f, 0.0f);
            robj->autobreak = true;
            robj->horSpace = width-8;
            robj->align = nite::TextAlign::CENTER;
            if(isProperty){
                nite::setColor(0.1f, 0.1f, 0.1f, 1.0f);
                auto robj = this->font.draw("$"+nite::toStr(space.price), width * 0.5f, height * 0.85f, 0.5f, 0.5f, 0.0f);
                robj->autobreak = true;
                robj->horSpace = width-4;
                robj->align = nite::TextAlign::CENTER;
            }
        spaceb.end();
        spaceb.flush();
        // append to board's batch
        float rotw = width * nite::cos(nite::toRadians(layout.orientation)) - height * nite::sin(nite::toRadians(layout.orientation));
        float roth = width * nite::sin(nite::toRadians(layout.orientation)) + height * nite::cos(nite::toRadians(layout.orientation));
        float x = (this->unitSize.width * layout.x) + nite::abs(rotw)*0.5;
        float y = (this->unitSize.height * layout.y) + nite::abs(roth)*0.5;
        batch.begin();
            nite::setColor(1.0f, 1.0f, 1.0f, 1.0f);
            spaceb.draw(x, y, spaceb.getSize().x, spaceb.getSize().y, 0.5f, 0.5f, layout.orientation);
        batch.end();
        batch.flush(false); // false doesn't clear batch
        return spaceb;
    };

    // generate board
    batch.begin();
        nite::setColor(bg);
        empty.draw(0, 0, width, height, 0.0f, 0.0f, 0.0f);
    batch.end();
    batch.flush(false);    
    for(auto &it : gType->layout){
        auto &space = gType->spaces[it.first];
        generateSpace(space, it.second, this->batch);
    }    
}

Board::Board(GameState *state){
    this->game = state;
}

void Board::swapPiece(UInt8 id, UInt8 to){
    auto &piece = this->pieces[id];
    auto &client = game->net->clients[id];

    auto findInSpace = [&](UInt8 id, const Vector<UInt8> &list){
        for(int i = 0; i < list.size(); ++i){
            if(list[i] == id){
                return i;
            }
        }
        return -1;
    };

    auto &oldSpace = occupied[client->currentSpace];
    auto &newSpace = occupied[to];
    auto chkOld = findInSpace(id, oldSpace);
    if(chkOld != -1){
        oldSpace.erase(oldSpace.begin() + chkOld);
    }
    client->currentSpace = to;
    newSpace.push_back(id);
}

void Board::update(){
    // if(nite::mouseWheel() == nite::mouseWheelUp){
    //     width = nite::round((float)width * 1.1f);
    //     height = nite::round((float)height * 1.1f);
    //     rerender();
    // }else
    // if(nite::mouseWheel() == nite::mouseWheelDown){
    //     width = nite::round((float)width * 0.9f);
    //     height = nite::round((float)height * 0.9f);       
    //     rerender();
    // }


        //    if(!this->game->net->isServer){
        //         nite::print("xD");
        //     }
            
    for(auto &it : game->net->clients){
        auto &client = it.second;
        auto pIt = pieces.find(it.second->id);
        Shared<Piece> piece;
        if(pIt == pieces.end()){
            piece = std::make_shared<Piece>(Piece());
            piece->owner = it.second;
            piece->color = it.second->color;
            piece->size = nite::Vec2(24.0f);
            piece->targetSize = piece->size;
            piece->originalSize = piece->size;            
            piece->texture = empty;
            this->pieces[client->id] = piece;
            setPieceAt(client->id, client->currentSpace);
        }else{
            piece = pIt->second;
        }
        
        if(client->route.size() > 0 && nite::getTicks()-piece->lastPieceMove > 300){
            auto np = client->route[0];
            client->route.erase(client->route.begin());
            
            piece->nextPosition = this->spaces[np];
            swapPiece(client->id, np);
 
            piece->lastPieceMove = nite::getTicks();

        }

        float add = 0.0f;
        nite::Vec2 tSize = piece->originalSize;
        if(client->route.size() > 0 || nite::getTicks()-piece->lastPieceMove < 300){
            double t = (double)nite::getTicks()-(double)piece->lastPieceMove;
            double diff = 1.0f;

            if(t < 150){
                diff = t / 150.f;
            }else
            if(t > 150 && t < 300){
                diff = 1.0 - ((t-150) / 150.0);
            }else{
                diff = 0.0f;
            }
            
            add = 64.0f*diff;
        }

        float xcenter = (float)width*0.5f;
        float ycenter = (float)height*0.5f;
        auto &cp = this->spaces[client->currentSpace];
        float mod = nite::sqrt(nite::pow(xcenter-cp.x, 2) + nite::pow(ycenter-cp.y, 2));
        float ang = nite::arctan(cp.y - ycenter, cp.x - xcenter);   
        if(client->route.size() > 0 || nite::getTicks()-piece->lastPieceMove < 300){
            mod -= 32.0f;
        }

        nite::Vec2 rp(xcenter + nite::cos(ang)*(mod+add), ycenter + nite::sin(ang)*(mod+add));


        piece->position.lerpDiscrete(rp, 0.15f);
        piece->size.lerpDiscrete(tSize, 0.35f);

    }

    
}

void Board::setPieceAt(UInt8 id, UInt8 space){
    auto pIt = pieces.find(id);
    auto clIt = game->net->clients.find(id);
    if(pIt == pieces.end()){
        nite::print("failed to place piece: piece id '"+nite::toStr(id)+"' doesn't exist");
        return;
    }
    if(clIt == game->net->clients.end()){
        nite::print("failed to place piece: client id '"+nite::toStr(id)+"' doesn't exist");
        return;
    }    
    auto &piece = pIt->second;
    auto &client = clIt->second;
    swapPiece(id, space);
    piece->position.set(this->spaces[space]);
    piece->nextPosition = piece->position;
}

void Board::draw(float x, float y){
    nite::setDepth(nite::DepthMiddle);
    nite::setColor(1.0f, 1.0f, 1.0f, 1.0f);
    nite::setRenderTarget(nite::RenderTargetGame);
    batch.draw(x, y, batch.getSize().x, batch.getSize().y, 0.0f, 0.0f, 0.0f);
    for(auto &it : pieces){
        auto &piece = it.second;
        piece->draw();
    }
}

void Piece::draw(){
    nite::setDepth(nite::DepthMiddle);
    nite::setColor(0.0f, 0.0f, 0.0f, 0.70f);
    auto objShadow = this->texture.draw(position.x + 3, position.y + 3, size.x, size.y, 0.5f, 0.5f, 0.0f);
    if(objShadow){
        objShadow->smooth = true;
    }
    nite::setColor(color);
    texture.draw(position.x, position.y, size.x, size.y, 0.5f, 0.5f, 0.0f);
}