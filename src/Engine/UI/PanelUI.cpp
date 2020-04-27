#include "PanelUI.hpp"

void nite::PanelUI::defaultInit(){
    size.set(16.0f);
    fillUpType = true;  
    componentName = "Panel"; 
    type = "panel";   
    flex = 0.0f;
    baseColor.set(1.0f, 1.0f, 1.0f, 0.0f);
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

void nite::PanelUI::rerender(){
    // if(!toRerender) return;  
    batch.begin();

    nite::setColor(baseColor);

    if(uiBackgroundImage.isLoaded()){
        auto *base = uiBackgroundImage.draw(0, 0, size.x, size.y, 0.0f, 0.0f, 0.0f);
        if(base != NULL){
            base->repeat = true;
        }
    }else{  
        auto ref = uiBasicTexture.draw(0.0f, 0.0f, size.x, size.y, 0.0f, 0.0f, 0.0f);
        if(ref != NULL && useShader){
            nite::Uniform uni;
            uni.add("size", size);
            uni.add("color", baseColor);
            uni.add("alpha", baseColor.a);
            uni.add("thickness", 5.0f);
            ref->apply(shader, uni);
        }        
    }

    // Render Children
    for(int i = 0; i < children.size(); ++i){

        auto cps = children[i]->computeSize();
        auto tcps = computeSize();

        if(children[i]->position.x < 0 || children[i]->position.y < 0 || !children[i]->visible){
            continue;
        }
        scrollOffsetTrans.lerpDiscrete(scrollOffset, 0.004f);
        auto usoffset = scrollX || scrollY ? scrollOffsetTrans : nite::Vec2(0.0f);
        children[i]->beforeRender();
        children[i]->render(usoffset);
        children[i]->afterRender();
        if(nav.enable && nav.index == children[i]->nav.index){
            // TODO: move this to an appropriate place
            static nite::Shader marker("data/shaders/ui/ui_marker_f.glsl", "data/shaders/ui/ui_marker_v.glsl");
            nite::setColor(nav.a);
            auto ref = uiBasicTexture.draw(children[i]->position.x + usoffset.x, children[i]->position.y + usoffset.y, children[i]->size.x, children[i]->size.y, 0.5f, 0.5f, 0.0f);
            // TODO: Scroll bar
            if(ref != NULL){
                auto uni = nite::Uniform();
                uni.add("size", size);
                uni.add("color", nav.color);
                uni.add("alpha", nav.color.a);
                uni.add("thickness", 5.0f);
                ref->apply(marker, uni);
            }
        }        
    }
    batch.end();
    batch.flush();
    // toRerender = false;
}

void nite::PanelUI::recalculateFromHead(){
    auto *head = getHeadComponent();
    if(head != this){
        head->recalculate(); 
    }
}

void nite::PanelUI::recalculate(){
    batch.init(size.x, size.y);
    layout->recalculate(*this);
    rerender(); 
    recalculateFromHead(); 
}

nite::PanelUI::PanelUI(const nite::Vec2 &size){
    defaultInit();
    this->size.set(size);
}

nite::PanelUI::PanelUI(){
    defaultInit();
}

void nite::PanelUI::setSize(const nite::Vec2 &size){
    this->size.set(size);
    recalculate();
}

nite::Vec2 nite::PanelUI::computeSize(){
    return size + padding + margin;
}

void nite::PanelUI::onCreate(){
    uiBasicTexture.load("data/sprite/empty.png");
    recalculate();
}

void nite::PanelUI::update(){
    // if(!realPosition.lerpDiscrete(position, componentTranslationSpeed)){
    //     recalculate();    
    // }    

    nav.update(this);

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

void nite::PanelUI::accommodate(float w, float h){
    if(nite::Vec2(w, h) != size){
        size.set(w, h);    
        recalculate();
    }else{
        size.set(w, h);      
    }
}

void nite::PanelUI::setBackgroundImage(const nite::Texture &bgi){
	uiBackgroundImage = bgi;
}

nite::Texture nite::PanelUI::getBackgroundImage(){
	return uiBackgroundImage;	
}

void nite::PanelUI::render(const nite::Vec2 &offset){

    auto cps = computeSize();
    nite::Vec2 rp = position - cps * 0.5f + margin * 0.5f + offset; 
    // Render batch
    nite::setRenderTarget(renderOnTarget);
    nite::setDepth(nite::DepthMiddle);

    nite::setColor(1.0f, 1.0f, 1.0f, 1.0f);
    batch.draw(rp.x, rp.y, size.x, size.y, 0.0f, 0.0f, 0.0f);
}

void nite::PanelUI::setBackgroundColor(const nite::Color &color){
    baseColor.set(color);
    recalculate();  
}

nite::Color nite::PanelUI::getBackgroundColor(){
    return baseColor;
}

void nite::PanelUI::onClick(){
    this->onClickMethod(Shared<nite::ListenerInfo>(new nite::ListenerInfo()), this);
}

void nite::PanelUI::onHover(){
    this->onHoverMethod(Shared<nite::ListenerInfo>(new nite::ListenerInfo()), this);
}

void nite::PanelUI::onUnhover(){
    this->onUnhoverMethod(Shared<nite::ListenerInfo>(new nite::ListenerInfo()), this);
}

void nite::PanelUI::setOnHover(nite::ListenerLambda onHover){
    this->onHoverMethod = onHover;
}

void nite::PanelUI::setOnClick(nite::ListenerLambda onClick){
    this->onClickMethod = onClick;
}

void nite::PanelUI::setOnUnhover(nite::ListenerLambda onUnhover){
    this->onUnhoverMethod = onUnhover;  
}
