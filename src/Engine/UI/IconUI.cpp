#include "IconUI.hpp"

void nite::IconUI::defaultInit(){
    size.set(32.0f);
    fillUpType = true;  
    componentName = "Icon";    
    flex = 1.0f;
    index = -1;
    iconSize.set(32.0f);
    baseColor.set(1.0f, 1.0f, 1.0f, 0.0f);
    onClickMethod = [](const Shared<nite::ListenerInfo> &info, nite::BaseUIComponent &panel){
        return;
    };
    onHoverMethod = [](const Shared<nite::ListenerInfo> &info, nite::BaseUIComponent &panel){
        return;
    };
    onUnhoverMethod = [](const Shared<nite::ListenerInfo> &info, nite::BaseUIComponent &panel){
        return;
    };    
}

void nite::IconUI::rerender(){
    // if(!toRerender) return;  
    // batch.begin();



    // Render Children
    // for(int i = 0; i < children.size(); ++i){
    //     if(children[i]->position.x < 0 || children[i]->position.y < 0 || !children[i]->visible){
    //         continue;
    //     }
    //     children[i]->beforeRender();
    //     children[i]->render();
    //     children[i]->afterRender();
    // }
    // batch.end();
    // batch.flush();
    // toRerender = false;
}

void nite::IconUI::recalculateFromHead(){
    auto *head = getHeadComponent();
    if(head != this){
        head->recalculate(); 
    }
}

void nite::IconUI::recalculate(){
    batch.init(size.x, size.y);
    layout->recalculate(*this);
    rerender(); 
    recalculateFromHead(); 
}

nite::IconUI::IconUI(const nite::Vec2 &size, const nite::Vec2 &iconSize, const nite::Texture &source, int index){
    defaultInit();
    this->size.set(size);
    this->iconSize = iconSize;
    this->source = source;
    this->index = index;
}

nite::IconUI::IconUI(){
    defaultInit();
}

void nite::IconUI::setSize(const nite::Vec2 &size){
    this->size.set(size);
    recalculate();
}

nite::Vec2 nite::IconUI::computeSize(){
    return size + padding + margin;
}

void nite::IconUI::onCreate(){
    empty.load("data/sprite/empty.png");
    recalculate();
}

void nite::IconUI::update(){
    if(!realPosition.lerpDiscrete(position, componentTranslationSpeed)){
        recalculate();    
    }    
    nite::Vec2 p = realPosition - computeSize() * 0.5f;
    for(int i = 0; i < children.size(); ++i){
        if(!uninteract){
            children[i]->updateRelativePosition(relativePosition + p);
            children[i]->updateListeners();
        }
        children[i]->beforeUpdate();
        children[i]->update();
        children[i]->afterUpdate();
    }
}

void nite::IconUI::accommodate(float w, float h){
    if(nite::Vec2(w, h) != size){
        size.set(w, h);    
        recalculate();
    }else{
        size.set(w, h);      
    }
}

nite::Vec2 nite::IconUI::getIconSize(){
    return iconSize;
}

void nite::IconUI::setIconSize(const nite::Vec2 &iconSize){
    this->iconSize.set(iconSize);
    recalculate();
}

void nite::IconUI::setIndex(int index){
    this->index = index;
    recalculate();
}

int nite::IconUI::getIndex(){
    return this->index;
}

void nite::IconUI::setSource(const nite::Texture &bgi){
	source = bgi;
    recalculate();
}

nite::Texture nite::IconUI::getSource(){
	return source;	
}

void nite::IconUI::render(){

    auto cps = computeSize();
    nite::Vec2 rp = realPosition - cps * 0.5f + margin * 0.5f;
    // Render batch
    nite::setRenderTarget(renderOnTarget);
    nite::setDepth(nite::DepthMiddle);

    nite::setColor(baseColor);
    if(source.isLoaded() && index > -1){
        int tx = source.getWidth() / iconSize.x;
        int ty = source.getHeight() / iconSize.y;
        int x = index % tx;
        int y = index / ty;
        this->iconPosition.set(x, y);
        source.setRegion(x * iconSize.x, y * iconSize.y, iconSize.x, iconSize.y);
        source.draw(rp.x, rp.y, size.x, size.y, 0.0f, 0.0f, 0.0f);         
    }else{
        empty.draw(rp.x, rp.y, size.x, size.y, 0.0f, 0.0f, 0.0f);
    }
}

void nite::IconUI::setBackgroundColor(const nite::Color &color){
    baseColor.set(color);
    recalculate();  
}

nite::Color nite::IconUI::getBackgroundColor(){
    return baseColor;
}

void nite::IconUI::onClick(){
    this->onClickMethod(Shared<nite::ListenerInfo>(new nite::ListenerInfo()), *this);
}

void nite::IconUI::onHover(){
    this->onHoverMethod(Shared<nite::ListenerInfo>(new nite::ListenerInfo()), *this);
}

void nite::IconUI::onUnhover(){
    this->onUnhoverMethod(Shared<nite::ListenerInfo>(new nite::ListenerInfo()), *this);
}

void nite::IconUI::setOnHover(nite::ListenerLambda onHover){
    this->onHoverMethod = onHover;
}

void nite::IconUI::setOnClick(nite::ListenerLambda onClick){
    this->onClickMethod = onClick;
}

void nite::IconUI::setOnUnhover(nite::ListenerLambda onUnhover){
    this->onUnhoverMethod = onUnhover;  
}
