#include "ProgressiveBarUI.hpp"

void nite::ProgressiveBarUI::defaultInit(){
    size.set(16.0f);
    fillUpType = true;  
    componentName = "Progressive Bar"; 
    type = "progressive-bar";   
    flex = 0.0f;
    borderThickness = 0.0f;
    baseColor.set(1.0f, 1.0f, 1.0f, 0.0f);
    useReduce = true;
    showMax = true;
    hitTimeout = nite::getTicks();
    showValue = true;    
    onClickMethod = [](const Shared<nite::ListenerInfo> &info, nite::BaseUIComponent *panel){
        return;
    };
    onHoverMethod = [](const Shared<nite::ListenerInfo> &info, nite::BaseUIComponent *panel){
        return;
    };
    onUnhoverMethod = [](const Shared<nite::ListenerInfo> &info, nite::BaseUIComponent *panel){
        return;
    };    
}

void nite::ProgressiveBarUI::setBorderColor(const nite::Color &color){
	this->borderColor.set(color);
	recalculate();  
}

void nite::ProgressiveBarUI::setBorderThickness(float tn){
	borderThickness = tn;
	recalculate();  
}

void nite::ProgressiveBarUI::setValue(float tvalue){
    this->targetValue = tvalue;
    recalculate();  

}

void nite::ProgressiveBarUI::setValue(float tvalue, float maxValue){
    this->targetValue = tvalue;
    this->maxValue = maxValue;
    this->hitTimeout = nite::getTicks();
    recalculate();  
}

void nite::ProgressiveBarUI::setBarColor(const nite::Color &color){
    this->barColorFull.set(color);
    recalculate();  
}

void nite::ProgressiveBarUI::setReduceColor(const nite::Color &color){
    this->barColorReduce.set(color);
    recalculate(); 
}

void nite::ProgressiveBarUI::setUseReduce(bool v){
    useReduce = v;
    recalculate(); 
}

void nite::ProgressiveBarUI::setUseMax(bool v){
    showMax = v;
    recalculate();     
}

void nite::ProgressiveBarUI::setUseValue(bool v){
    showValue = v;
    recalculate();     
}

void nite::ProgressiveBarUI::rerender(){
    // if(!toRerender) return;  
    batch.begin();

    auto thickness = borderThickness * getGeneralScale();

    nite::Vec2 fsize = nite::Vec2(nite::floor(size.x), nite::floor(size.y));

    nite::setColor(borderColor);
    uiBasicTexture.draw(0.0f, 0.0f, fsize.x, fsize.y, 0.0f, 0.0f, 0.0f);

    nite::setColor(baseColor);
    uiBasicTexture.draw(thickness, thickness, nite::floor(fsize.x - thickness * 2.0f), nite::floor(fsize.y - thickness * 2.0f), 0.0f, 0.0f, 0.0f);

    // bar
    nite::setColor(useReduce ? barColorReduce : barColorFull);
    uiBasicTexture.draw(thickness, thickness, (float)nite::floor(fsize.x - thickness * 2.0f) * (currentValue / maxValue), nite::floor(fsize.y - thickness * 2.0f), 0.0f, 0.0f, 0.0f);
    if(useReduce){
        nite::setColor(barColorFull);
        uiBasicTexture.draw(thickness, thickness, (float)nite::floor(fsize.x - thickness * 2.0f) * (targetValue / maxValue), nite::floor(fsize.y - thickness * 2.0f), 0.0f, 0.0f, 0.0f);
    }

    batch.end();
    batch.flush();    
    // toRerender = false;
}

void nite::ProgressiveBarUI::recalculateFromHead(){
    auto *head = getHeadComponent();
    if(head != this){
        head->recalculate(); 
    }
}

void nite::ProgressiveBarUI::recalculate(){
    batch.init(size.x, size.y);
    layout->recalculate(*this);
    rerender(); 
    recalculateFromHead(); 
}

nite::ProgressiveBarUI::ProgressiveBarUI(const nite::Vec2 &size){
    defaultInit();
    this->size.set(size);
}

nite::ProgressiveBarUI::ProgressiveBarUI(){
    defaultInit();
}

void nite::ProgressiveBarUI::setSize(const nite::Vec2 &size){
    this->size.set(size);
    recalculate();
}

nite::Vec2 nite::ProgressiveBarUI::computeSize(){
    return size + margin;
}

void nite::ProgressiveBarUI::onCreate(){
    if(!uiShader.isLoaded()){
        uiShader.load("data/shaders/basic_ui_background_f.glsl", "data/shaders/basic_ui_background_v.glsl");
    }
    if(!uiBasicTexture.isLoaded()){
        uiBasicTexture.load("data/texture/empty.png");
    }
    if(!font.isLoaded()){
        font.load(defaultFontUI, fontSize * defaultFontRatio, 0.0f);  
    }

    recalculate();
}

void nite::ProgressiveBarUI::setFontSize(int size){
    if(size <= 0) return;
    this->fontSize = size;
    auto fn = font.getFilename();
    if(fn.length() > 0){
        font.load(fn, size, font.getTickness());
    }
    calculateSize();  
    recalculate();
}

void nite::ProgressiveBarUI::setFont(const nite::Font &font){
    this->font = font;
    this->fontSize = this->font.getFontSize();
    calculateSize();  
    recalculate();
}

void nite::ProgressiveBarUI::update(){
    if(!visible){
        return;
    }
    if(!realPosition.cInterpDiscrete(position, 1.0f)){
        recalculate();    
    }    

    if(useReduce && nite::getTicks()-hitTimeout > 1000 || !useReduce){
        if(!nite::lerpDiscrete(currentValue, targetValue, 0.1f, 0.1)){
            recalculate();    
        }
    }

    nite::Vec2 p = position - computeSize() * 0.5f;
    for(int i = 0; i < children.size(); ++i){
        if(!uninteract){
            children[i]->updateRelativePosition(relativePosition + p + scrollOffset);
            children[i]->updateListeners();
        }
        children[i]->beforeUpdate();
        children[i]->update();
        children[i]->afterUpdate();
    }
}

void nite::ProgressiveBarUI::accommodate(float w, float h){
    if(nite::Vec2(w, h) == size){
        return;
    }  
    size.set(w, h);   
    recalculate();
}

void nite::ProgressiveBarUI::setBackgroundImage(const nite::Texture &bgi){
	uiBackgroundImage = bgi;
}

nite::Texture nite::ProgressiveBarUI::getBackgroundImage(){
	return uiBackgroundImage;	
}

void nite::ProgressiveBarUI::render(const nite::Vec2 &offset){

    auto cps = computeSize();
    nite::Vec2 rp = realPosition - cps * 0.5f + margin * 0.5f + offset; 
    // Render batch
    nite::setRenderTarget(renderOnTarget);
    nite::setDepth(nite::DepthMiddle);

    nite::setColor(1.0f, 1.0f, 1.0f, 1.0f);
    auto ref = batch.draw(rp.x, rp.y, size.x, size.y, 0.0f, 0.0f, 0.0f);
    if(ref != NULL){
        ref->smooth = false;
    }
    if(ref != NULL && useShader){
        nite::Uniform uni = this->uniform; // make a copy
        uni.add("p_size", size);
        // if(uni.colors.find("p_color") == uni.colors.end()){
        //     uni.add("p_color", nite::Color(1.0f, 1.0f, 1.0f));
        // }
        ref->apply(shader, uni);
    } 
}

void nite::ProgressiveBarUI::setBackgroundColor(const nite::Color &color){
    if(baseColor == color){
        return;
    }
    baseColor.set(color);
    recalculate();  
}

nite::Color nite::ProgressiveBarUI::getBackgroundColor(){
    return baseColor;
}

void nite::ProgressiveBarUI::onClick(){
    this->onClickMethod(Shared<nite::ListenerInfo>(new nite::ListenerInfo()), this);
}

void nite::ProgressiveBarUI::onHover(){
    this->onHoverMethod(Shared<nite::ListenerInfo>(new nite::ListenerInfo()), this);
}

void nite::ProgressiveBarUI::onUnhover(){
    this->onUnhoverMethod(Shared<nite::ListenerInfo>(new nite::ListenerInfo()), this);
}

void nite::ProgressiveBarUI::setOnHover(nite::ListenerLambda onHover){
    this->onHoverMethod = onHover;
}

void nite::ProgressiveBarUI::setOnClick(nite::ListenerLambda onClick){
    this->onClickMethod = onClick;
}

void nite::ProgressiveBarUI::setOnUnhover(nite::ListenerLambda onUnhover){
    this->onUnhoverMethod = onUnhover;  
}
