#include "TextUI.hpp"

nite::TextUI::TextUI(){
    defaultInit();
}

nite::TextUI::TextUI(const String &text){
    defaultInit();
    this->text = text;  
    this->originalText = text;
}

void nite::TextUI::defaultInit(){
    fontSize = 14;
    text = "";
    originalText = "";
    realSize.set(-1.0f);  
    size.set(-1.0f);
    type = "text";
    flex = 0.0f;
    fillUpType = true;
    componentName = "Text";    
    shadowOffset.set(1.0f);
    shadowColor.set(0.0f, 0.0f, 0.0f, 0.0f);
    baseColor.set(1.0f, 1.0f, 1.0f, 1.0f);
    onClickMethod = [](const Shared<nite::ListenerInfo> &info, nite::BaseUIComponent *text){
        return;
    };
    onHoverMethod = [](const Shared<nite::ListenerInfo> &info, nite::BaseUIComponent *text){
        return;
    };
    onUnhoverMethod = [](const Shared<nite::ListenerInfo> &info, nite::BaseUIComponent *text){
        return;
    };    
}

void nite::TextUI::accommodate(float w, float h){
    // fix
    nite::Vec2 left(w, h);
    auto _reset = [&](){
    left.x = w;
    };
    text = "";
    float cw = 0.0f;
    auto getWordWidth = [&](const String &word){
        float _w = 0.0f;
        for(int i = 0; i < word.size(); ++i){
            _w += font.getWidth(String("")+word[i]);
        }
        return _w;
    };
    auto words = nite::split(originalText, ' ');
    for(int i = 0; i < words.size(); ++i){
        auto word = words[i] + " ";
        float ww = getWordWidth(word);
        if(ww > left.x){
            _reset();
            text += '\n';
        }
        left.x -= ww;    
        text += word;
    }
    // for(int i = 0; i < originalText.length(); ++i){
    //   float cw = font.getWidth(String("")+originalText[i]);
    //   left.x -= cw;     
    //   if(left.x < 0){
    //     _reset();
    //     text += '\n';
    //   }
    //   text += originalText[i];
    // }
    calculateSize();
}

void nite::TextUI::onCreate(){
    if(!font.isLoaded()){
        font.load(defaultFontUI, fontSize * defaultFontRatio, 1.0f);  
    }
}

void nite::TextUI::setShadowColor(const nite::Color &color){
    this->shadowColor.set(color);
}

void nite::TextUI::setShadowOffset(const nite::Vec2 &offset){
    this->shadowOffset.set(offset);
}

nite::Color nite::TextUI::getShadowColor(){
    return shadowColor;
}

nite::Vec2 nite::TextUI::getShadowOffset(){
    return shadowOffset;
}

void nite::TextUI::setFontSize(int size){
    if(size <= 0) return;
    this->fontSize = size;
    auto fn = font.getFilename();
    if(fn.length() > 0){
        font.load(fn, size, 1.0f);
    }
    calculateSize();  
    recalculate();
}

int nite::TextUI::getFontSize(){
    return font.getFontSize();
}

String nite::TextUI::getFontFilename(){
    return font.getFilename();
}

void nite::TextUI::setFont(const nite::Font &font){
    this->font = font;
    this->fontSize = this->font.getFontSize();
    calculateSize();  
    recalculate();
}

void nite::TextUI::setText(const String &text){
    if(text == this->text){
        return;
    }
    this->text = text;
    this->originalText = text;
    calculateSize();  
    recalculate();
}

String nite::TextUI::getText(){
    return this->text;
}

void nite::TextUI::setOnClick(nite::ListenerLambda onClick){
    this->onClickMethod = onClick;
}

void nite::TextUI::setOnHover(nite::ListenerLambda onHover){
    this->onHoverMethod = onHover;
}

void nite::TextUI::setOnUnhover(nite::ListenerLambda onUnhover){
    this->onUnhoverMethod = onUnhover;
}

void nite::TextUI::onHover(){
    this->onHoverMethod(Shared<nite::ListenerInfo>(new nite::ListenerInfo()), this);
}

void nite::TextUI::onClick(){
    this->onClickMethod(Shared<nite::ListenerInfo>(new nite::ListenerInfo()), this);
}

void nite::TextUI::onUnhover(){
    this->onUnhoverMethod(Shared<nite::ListenerInfo>(new nite::ListenerInfo()), this);
}

void nite::TextUI::calculateSize(){
    size.x = realSize.x == -1.0f ? font.getWidth(text) : realSize.x;
    size.y = realSize.y == -1.0f ? font.getHeight() : realSize.y;
}

nite::Vec2 nite::TextUI::computeSize(){
    return nite::Vec2(font.getWidth(text), font.getHeight()) + margin + padding;
}

void nite::TextUI::setFontColor(const nite::Color &color){
    baseColor.set(color);
    recalculate();
}

nite::Color nite::TextUI::getFontColor(){
    return baseColor;
}

// void nite::TextUI::setSize(const nite::Vec2 &size){
//     this->realSize.set(size);
//     calculateSize();
//     recalculate();  
// }

void nite::TextUI::render(const nite::Vec2 &offset){
    nite::setColor(baseColor);
    auto cps = computeSize();
    nite::Vec2 p(position.x + padding.x * 0.5f - cps.x * 0.5f, position.y + padding.y * 0.5f - cps.y * 0.5f);
    p = p + offset;
    auto *ref = font.draw(text, p.x, p.y, 0.0f, 0.0f, 0.0f);
    if(ref != NULL && shadowColor.a > 0.0f){
        ref->setShadow(shadowColor, shadowOffset);
    }  
}

void nite::TextUI::update(){
    // if(!realPosition.cInterpDiscrete(position, componentTranslationSpeed)){
    // // recalculate();    
    // }   
}