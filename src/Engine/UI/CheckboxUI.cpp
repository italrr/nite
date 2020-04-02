#include "CheckboxUI.hpp"

nite::CheckboxUI::CheckboxUI(){
    defaultInit();
}

nite::CheckboxUI::CheckboxUI(const String &text, nite::ListenerLambda onChange){
    defaultInit();
    this->text = text;
    this->onChangeMethod = onChange;
}

void nite::CheckboxUI::defaultInit(){
    fontSize = 14;
    text = "";
    resetColor();
    realSize.set(-1.0f);
    fillUpType = false;
    flex = 0.0f;
    type = "checkbox";
    value = false;
    componentName = "Checkbox";  
    padding.set(8.0f);
    onChangeMethod = [](const Shared<nite::ListenerInfo> &info, nite::BaseUIComponent &checkbox){
        return;
    };
    onHoverMethod = [](const Shared<nite::ListenerInfo> &info, nite::BaseUIComponent &checkbox){
        return;
    };   
    onUnhoverMethod = [](const Shared<nite::ListenerInfo> &info, nite::BaseUIComponent &checkbox){
        return;
    };   
}

void nite::CheckboxUI::resetColor(){
    selectorColor.set(1.0f, 1.0f, 1.0f, 1.0f);
    baseColor.set(0.1f, 0.1f, 0.1f, 1.0f);  
    textColor.set(1.0f, 1.0f, 1.0f, 1.0f);
    recalculate();
}

void nite::CheckboxUI::setOnChange(nite::ListenerLambda onChange){
    this->onChangeMethod = onChange;  
}

void nite::CheckboxUI::setOnHover(nite::ListenerLambda onHover){
    this->onHoverMethod = onHover;
}

void nite::CheckboxUI::setOnUnhover(nite::ListenerLambda onUnhover){
    this->onUnhoverMethod = onUnhover;  
}

void nite::CheckboxUI::calculateSize(){
    size.x = realSize.x == -1.0f ? font.getWidth(text) + font.getHeight() : realSize.x;
    size.y = realSize.y == -1.0f ? font.getHeight() : realSize.y;
}

void nite::CheckboxUI::setFont(const nite::Font &font){
    this->font = font;
    this->fontSize = this->font.getFontSize();
    calculateSize();
    recalculate();
}

void nite::CheckboxUI::onCreate(){
    font.load(defaultFontUI, fontSize * defaultFontRatio, 1.0f); 
    calculateSize();  
}

void nite::CheckboxUI::setText(const String &text){
    this->text = text;
    calculateSize();
    recalculate();
}

void nite::CheckboxUI::setFontSize(int size){
    if(size <= 0) return;
    this->fontSize = size;
    auto fn = font.getFilename();
    if(fn.length() > 0){
        font.load(fn, size);
    }
    calculateSize();  
    recalculate(); 
}

nite::Vec2 nite::CheckboxUI::computeSize(){
    return size + nite::Vec2(4.0f, 0.0f) + margin + padding + nite::Vec2(2.0f);
}

void nite::CheckboxUI::render(){
    static nite::Texture blank("data/sprite/empty.png");
    nite::Vec2 p(position.x, position.y);
    float boxdim = font.getHeight();
    auto cs = size + padding;
    nite::Vec2 rp(cs * 0.5f);
    nite::setColor(baseColor);
    blank.draw(p.x - rp.x, p.y - rp.y, boxdim, boxdim, 0.0f, 0.0f, 0.0f);
    if(value){
        nite::setColor(selectorColor);    
        blank.draw(p.x - rp.x + boxdim * 0.25f, p.y - rp.y + boxdim * 0.25f, boxdim * 0.5f, boxdim * 0.5f, 0.0f, 0.0f, 0.0f);
    }
    nite::setColor(textColor);
    auto *ref = font.draw(text, p.x - rp.x + boxdim + 4.0f, p.y - rp.y + boxdim * 0.15f, 0.0f, 0.0f, 0.0f);
}

void nite::CheckboxUI::update(){
    // animations
    // if(!realPosition.lerpDiscrete(position, componentTranslationSpeed)){
    //     recalculate();    
    // }
}

nite::Color nite::CheckboxUI::getBoxColor(){
    return baseColor;
}

nite::Color nite::CheckboxUI::getFontColor(){
    return textColor;
}

nite::Color nite::CheckboxUI::getSelectorColor(){
    return selectorColor;
}

void nite::CheckboxUI::setBoxColor(const nite::Color &color){
    this->baseColor.set(color);
    recalculate();  
}

void nite::CheckboxUI::setFontColor(const nite::Color &color){
    this->textColor.set(color);
    recalculate();  
}

void nite::CheckboxUI::setSelectorColor(const nite::Color &color){
    this->selectorColor.set(color);
    recalculate();
}

void nite::CheckboxUI::setSize(const nite::Vec2 &size){
    this->realSize.set(size);
    calculateSize(); 
    recalculate();
}

void nite::CheckboxUI::onClick(){
    value = !value;
    onChangeMethod(Shared<nite::ListenerInfoCheckbox>(new nite::ListenerInfoCheckbox(value)), *this);
    recalculate();
} 

void nite::CheckboxUI::onHover(){
    onHoverMethod(Shared<nite::ListenerInfo>(new nite::ListenerInfo()), *this);
}

void nite::CheckboxUI::onUnhover(){
    onUnhoverMethod(Shared<nite::ListenerInfo>(new nite::ListenerInfo()), *this);
}

int nite::CheckboxUI::getFontSize(){
    return font.getFontSize();
}

String nite::CheckboxUI::getFontFilename(){
    return font.getFilename();
}