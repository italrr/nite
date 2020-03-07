#include "IconUI.hpp"

void nite::IconUI::defaultInit(){
    size.set(32.0f);
    fillUpType = true;  
    componentName = "Icon";    
    fontSize = 14;
    flex = 0.0f;
    index = -1;
    textPosition.set(0.5f);
    textColor.set(0.0f, 0.0f, 0.0f, 1.0f);
    iconSize.set(32.0f);
    baseColor.set(1.0f, 1.0f, 1.0f, 0.0f);
    shadowOffset.set(1.0f);
    shadowColor.set(0.0f, 0.0f, 0.0f, 0.0f);    
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
    batch.begin();

    nite::setColor(baseColor.r, baseColor.g, baseColor.b, 1.0f);
    if(source.isLoaded() && index > -1){
        int tx = source.getWidth() / iconSize.x;
        int ty = source.getHeight() / iconSize.y;
        int x = index % tx;
        int y = index / ty;
        this->iconPosition.set(x, y);
        source.setRegion(x * iconSize.x, y * iconSize.y, iconSize.x, iconSize.y);
        source.draw(0.0f, 0.0f, size.x, size.y, 0.0f, 0.0f, 0.0f);         
    }else{
        empty.draw(0.0f, 0.0f, size.x, size.y, 0.0f, 0.0f, 0.0f);
    }

    if(text.length() > 0){
        nite::setColor(textColor);
        nite::Vec2 p = textPosition * size; // position it's actually a percentange
        auto ref = font.draw(text, 0.0f + p.x, 0.0f + p.y, 0.5f, 0.5f, 0.0f);
        if(ref != NULL && shadowColor.a > 0.0f){
            ref->setShadow(shadowColor, shadowOffset);
        }          
    }

    // Render Children
    for(int i = 0; i < children.size(); ++i){
        if(children[i]->position.x < 0 || children[i]->position.y < 0 || !children[i]->visible){
            continue;
        }
        children[i]->beforeRender();
        children[i]->render();
        children[i]->afterRender();
    }
    batch.end();
    batch.flush();
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

void nite::IconUI::setShadowColor(const nite::Color &color){
    this->shadowColor.set(color);
    calculateSize();  
    recalculate();     
}

nite::Color nite::IconUI::getShadowColor(){
    return this->shadowColor;
}

void nite::IconUI::setShadowOffset(const nite::Vec2 &offset){
    this->shadowOffset.set(offset);
    calculateSize();  
    recalculate();   
}

nite::Vec2 nite::IconUI::getShadowOffset(){
    return shadowOffset;
}

void nite::IconUI::setFontSize(int s){
    if(s <= 0) return;
    this->fontSize = s;
    auto fn = font.getFilename();
    if(fn.length() > 0){
        font.load(fn, s);
    }
    calculateSize();  
    recalculate();
}

int nite::IconUI::getFontSize(){
    return fontSize;
}

void nite::IconUI::setFont(const nite::Font &font){
    this->font = font;
    calculateSize();  
    recalculate();
}

nite::Font nite::IconUI::getFont(){
    return font;
}

nite::Color nite::IconUI::getTextColor(){
    return textColor;
}

void nite::IconUI::setTextColor(const nite::Color &c){
    this->textColor.set(c);
    calculateSize();  
    recalculate();
}

void nite::IconUI::setText(const String &text){
    this->text = text;
    calculateSize();  
    recalculate();
}

String nite::IconUI::getText(){
    return text;
}

void nite::IconUI::setTextPosition(const nite::Vec2 &p){
    this->textPosition.set(p);
    calculateSize();  
    recalculate();
}

nite::Vec2 nite::IconUI::getTextPosition(){
    return textPosition;
}

void nite::IconUI::setSize(const nite::Vec2 &size){
    this->size.set(size);
    calculateSize();  
    recalculate();
}

nite::Vec2 nite::IconUI::computeSize(){
    return size + padding + margin;
}

void nite::IconUI::onCreate(){
    font.load(defaultFontUI, fontSize * defaultFontRatio);  
    empty.load("data/sprite/empty.png");
    calculateSize();  
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

    nite::setColor(1.0f, 1.0f, 1.0f, 1.0f);
    batch.draw(rp.x, rp.y, size.x, size.y, 0.0f, 0.0f, 0.0f);
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
