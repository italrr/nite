#include "../Engine/Tools/Tools.hpp"
#include "Icon.hpp"

void Game::Icon::draw(float x, float y, float w, float h, float origx, float origy, float angle){
    if(id == -1 || source.isLoaded()){
        return;
    }
    source.setRegion(inTexPosition.x, inTexPosition.y, iconSize.x, iconSize.y);
    source.draw(x, y, w, h, origx, origy, angle);    
}

void Game::IconManager::load(const nite::Texture &source, nite::Vec2 iconSize){
    this->source = source;
    this->iconSize = iconSize;
    int totalW = this->source.getWidth() / iconSize.x;
    int totalH = this->source.getHeight() / iconSize.y;
    this->total = totalW * totalH;
    for(int i = 0; i < total; ++i){
        Game::Icon icon;
        int w = i % totalW;
        int h = i / totalW;
        icon.iconSize = iconSize;
        icon.inTexPosition.set(w, h);
        icon.id = i;
        icon.source = source;
        icons[i] = icon;
    }
    nite::print("icon manager: loaded "+nite::toStr(total)+" icons from '"+this->source.getFilename()+"'");
}

Game::Icon &Game::IconManager::getIcon(int id){
    auto it = icons.find(id);
    if(it == icons.end()){
        Game::Icon empty;
        empty.id = -1;
        return empty;
    }
    return icons[id];
}

Game::Icon &Game::IconManager::getIcon(int x, int y){
    return getIcon(x + y * iconSize.y);
}