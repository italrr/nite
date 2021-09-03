#include "ButtonUI.hpp"
#include "../Texture.hpp"

nite::ButtonUI::ButtonUI(){
    defaultInit();
}

void nite::ButtonUI::defaultInit(){
    fontSize = 14;
    text = "";
    realSize.set(-1.0f);
    resetColor();
    fillUpType = false;
    borderColor.set(nite::Color("#000000"));
    baseColor.set(nite::Color("#ff0000"));
    textColor.set(nite::Color("#000000"));
    componentName = "Button";
    type = "button";
    padding.set(0.0f);
    flex = 0.0f;
    pressOffset = 5.0f;
    currentPressOffset = 0.0f;
    // borderThickness.set(2.0f);
    borderThickness = 2.0f;
    onClickMethod = [](const Shared<nite::ListenerInfo> &info, nite::BaseUIComponent *button){
        return;
    };
    onHoverMethod = [](const Shared<nite::ListenerInfo> &info, nite::BaseUIComponent *button){
        return;
    };
    onUnhoverMethod = [](const Shared<nite::ListenerInfo> &info, nite::BaseUIComponent *button){
        return;
    };  
}

nite::ButtonUI::ButtonUI(const String &text, nite::ListenerLambda onClick){
    defaultInit();
    this->text = text;
    this->onClickMethod = onClick;
}

void nite::ButtonUI::setSize(const nite::Vec2 &size){
    this->realSize.set(size);
    calculateSize();
    recalculate();
}

void nite::ButtonUI::resetColor(){
    borderColor.set(nite::Color("#000000"));
    baseColor.set(nite::Color("#ff0000"));
    textColor.set(nite::Color("#000000"));
    recalculate();
}

void nite::ButtonUI::calculateSize(){
    if(!buttonImage.isLoaded()){
        size.x = realSize.x == -1.0f ? font.getWidth(text) : realSize.x;
        size.y = realSize.y == -1.0f ? font.getHeight() : realSize.y;
    }else{
        size.x = realSize.x == -1.0f ? 64 : realSize.x;
        size.y = realSize.y == -1.0f ? 24 :  realSize.y;
    }
}

void nite::ButtonUI::setOnClick(nite::ListenerLambda onClick){
    this->onClickMethod = onClick;  
}

void nite::ButtonUI::onCreate(){ 
    uiShader.load("data/shaders/basic_ui_background_f.glsl", "data/shaders/basic_ui_background_v.glsl");
    if(!font.isLoaded()){
        font.load(defaultFontUI, fontSize * defaultFontRatio, 0.0f);  
    }
    calculateSize();
}

void nite::ButtonUI::setFontSize(int size){
    if(size <= 0) return;
    this->fontSize = size;
    auto fn = font.getFilename();
    if(fn.length() > 0){
        font.load(fn, size, font.getTickness());
    }
    calculateSize();  
    recalculate();
}

int nite::ButtonUI::getFontSize(){
    return font.getFontSize();
}

String nite::ButtonUI::getFontFilename(){
    return font.getFilename();
}

void nite::ButtonUI::setFont(const nite::Font &font){
    this->font = font;
    this->fontSize = this->font.getFontSize();
    calculateSize();  
    recalculate();
}

void nite::ButtonUI::setBackgroundImage(const nite::Texture &image){
    this->buttonImage = image;
}

void nite::ButtonUI::setText(const String &text){
    this->text = text;
    this->buttonImage.unload();
    calculateSize();   
    recalculate();
}

nite::Color nite::ButtonUI::getBaseColor(){
    return baseColor;
}

nite::Color nite::ButtonUI::getBorderColor(){
    return borderColor;
}

nite::Color nite::ButtonUI::getFontColor(){
    return textColor;
}

void nite::ButtonUI::setBaseColor(const nite::Color &baseColor){
    this->baseColor.set(baseColor);
    recalculate();
}

void nite::ButtonUI::setFontColor(const nite::Color &baseColor){
    this->textColor.set(baseColor);
    recalculate();
}

nite::Vec2 nite::ButtonUI::computeSize(){
    return size + margin + padding;
}

void nite::ButtonUI::onClick(){
    buttonState = 2;
    stateTimeout = nite::getTicks();
}

void nite::ButtonUI::onUnhover(){
    this->onUnhoverMethod(Shared<nite::ListenerInfo>(new nite::ListenerInfo()), this);
}

void nite::ButtonUI::onHover(){
    this->onHoverMethod(Shared<nite::ListenerInfo>(new nite::ListenerInfo()), this);
    if(buttonState == 2) return;
    buttonState = 1; // on hover
    stateTimeout = nite::getTicks();
    calculateSize();   
    recalculate();    
}

void nite::ButtonUI::setOnHover(nite::ListenerLambda onHover){
    this->onHoverMethod = onHover;
}

void nite::ButtonUI::setOnUnhover(nite::ListenerLambda onUnhover){
    this->onUnhoverMethod = onUnhover;

}

void nite::ButtonUI::setBorderColor(const nite::Color &color){
	this->borderColor.set(color);
	recalculate();  
}

void nite::ButtonUI::setBorderThickness(float tn){
	borderThickness = tn;
	recalculate();  
}

void nite::ButtonUI::render(const nite::Vec2 &offset){
    static nite::Texture blank("data/texture/empty.png");
    nite::Vec2 p(position.x + offset.x, position.y + offset.y);
    // nite::print(padding);
    auto cs = size + padding;
    nite::Color bc = baseColor;
    nite::Color tc = textColor;
    if(buttonState == 1){
        bc = baseColor * 0.5f;
        tc = textColor * 0.5f;
    }
    if(buttonState == 2){
        bc = baseColor * 1.0f;
        tc = textColor * 1.0f;
    }
    // nite::Vec2 &bt = borderThickness;
    // auto bt = nite::Vec2(0.0f);
    nite::Vec2 rp(cs * 0.5f);
    nite::setColor(nite::Color(0.0f, 0.5f));
    // TODO: handle DPI
    blank.draw(p.x + 2, p.y + 2, cs.x, cs.y, 0.5f, 0.5f, 0.0f);
    nite::setColor(bc);  
    auto obj = blank.draw(p.x, p.y, cs.x, cs.y, 0.5f, 0.5f, 0.0f);
    if(obj != NULL){
        nite::Uniform uni;
        uni.add("p_size", size);
        uni.add("p_bcolor", bc);
        uni.add("p_lbcolor", secondColor);
        uni.add("p_salpha", secondColor.a);
        uni.add("p_alpha", secondColor.a);
        // nite::print(borderThickness);
        uni.add("p_hborder", nite::Vec2(borderThickness * getGeneralScale()));
        uni.add("p_vborder", nite::Vec2(borderThickness * getGeneralScale(), borderThickness * getGeneralScale()));     
        uni.add("p_blurthickness", 2.0f * (1.0f / ((size.avg() * getGeneralScale()) / nite::Vec2(nite::Vec2(300, 500)*getGeneralScale()).avg()))); // 300x500 was the original design size
        obj->apply(uiShader, uni);            
    }
    // nite::setColor(bc);  
    // blank.draw(p.x - rp.x + bt.x, p.y - rp.y + bt.y, cs.x - bt.x * 2.0f, cs.y - bt.y * 2.0f, 0.0f, 0.0f, 0.0f);
    //   blank.draw(p.x - rp.x, p.y - rp.y, cs.x, cs.y, 0.0f, 0.0f, 0.0f);  

    if(!buttonImage.isLoaded()){
        nite::setColor(tc);  
        auto *ref = font.draw(text, p.x, p.y, 0.5f, 0.5f, 0.0f);
        if(ref != NULL){
            ref->shadow = false;
            // nite::Color sc = bc.invert();
            // ref->setShadow(nite::Color(sc.r, sc.g, sc.b, 1.0f), nite::Vec2(1.0f));
        }  	
    }else{
        auto total = size.avg() * 0.65f;
        nite::Vec2 is(buttonImage.getSize());
        while(is.avg() > total){
            is.x *= 0.90f;
            is.y *= 0.90f;
        }
        nite::setColor(tc); 
        auto *ref = buttonImage.draw(p.x, p.y - is.y * 0.09f, is.x, is.y, 0.5f, 0.5f, 0.0f);
        if(ref != NULL){
            ref->smooth = true;
        }
    }
}

float nite::ButtonUI::getBorderThickness(){
	return borderThickness;
}

void nite::ButtonUI::update(){
    // if(!realPosition.cInterpDiscrete(position, componentTranslationSpeed)){
    //     recalculate();    
    // }
    if(!nite::cInterp(currentPressOffset, buttonState == 2 ? pressOffset : 0.0f, 0.70f)){
        rerender();
    }
    if(buttonState != 0){
        rerender();
        if(nite::getTicks()-stateTimeout > 35 * (buttonState == 2 ? 4 : 1)){
            if(buttonState == 2){
                this->onClickMethod(Shared<nite::ListenerInfo>(new nite::ListenerInfo()), this);    
            }
            buttonState = 0;
            rerender();
        }
    }
}