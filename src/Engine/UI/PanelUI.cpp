#include "PanelUI.hpp"

void nite::PanelUI::defaultInit(){
    size.set(16.0f);
    fillUpType = true;  
    componentName = "Panel"; 
    type = "panel";   
    flex = 0.0f;
    borderThickness = 0.0f;
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

void nite::PanelUI::setBorderColor(const nite::Color &color){
	this->borderColor.set(color);
	recalculate();  
}

void nite::PanelUI::setBorderThickness(float tn){
	borderThickness = tn;
	recalculate();  
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
        if(borderThickness > 0.0f && uiShader.isLoaded()){
            nite::Uniform uni;
            uni.add("p_size", size);
            uni.add("p_bcolor", baseColor);
            uni.add("p_lbcolor", borderColor);
            uni.add("p_salpha", borderColor.a);
            uni.add("p_alpha", baseColor.a);
            uni.add("p_hborder", nite::Vec2(borderThickness * getGeneralScale()));
            uni.add("p_vborder", nite::Vec2(borderThickness * getGeneralScale(), borderThickness * getGeneralScale()));     
            uni.add("p_blurthickness", 2.0f * (1.0f / ((size.avg() * getGeneralScale()) / nite::Vec2(nite::Vec2(300, 500)*getGeneralScale()).avg()))); // 300x500 was the original design size
            ref->apply(uiShader, uni);    
        } 
    }

    // Render Children
    for(int i = 0; i < children.size(); ++i){

        auto cps = children[i]->computeSize();
        auto tcps = computeSize();

        if(children[i]->position.x < 0 || children[i]->position.y < 0 || !children[i]->visible){
            continue;
        }
        scrollOffsetTrans.cInterpDiscrete(scrollOffset, 0.004f);
        auto usoffset = scrollX || scrollY ? scrollOffsetTrans : nite::Vec2(0.0f);
        children[i]->beforeRender();
        children[i]->render(usoffset);
        children[i]->afterRender();
        if(nav.enable && nav.current == children[i]->nav.index){
            // TODO: move this to an appropriate place
            static nite::Shader marker("data/shaders/ui/ui_marker_f.glsl", "data/shaders/ui/ui_marker_v.glsl");
            nite::setColor(nav.a);
            nite::Vec2 expSize = nite::Vec2(4.0f * (nav.expInc/100.0f));
            auto ref = uiBasicTexture.draw(children[i]->position.x + usoffset.x, children[i]->position.y + usoffset.y, cps.x + expSize.x, cps.y + expSize.y, 0.5f, 0.5f, 0.0f);
            // TODO: Scroll bar
            if(ref != NULL){
                auto uni = nite::Uniform();
                uni.add("size", size);
                uni.add("color", nav.color);
                uni.add("alpha", nav.color.a);
                uni.add("thickness", 5.0f );//* (1.0f + (nav.expInc/100.0f)));
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
    uiShader.load("data/shaders/basic_ui_background_f.glsl", "data/shaders/basic_ui_background_v.glsl");
    uiBasicTexture.load("data/texture/empty.png");
    recalculate();
}

void nite::PanelUI::update(){
    if(!visible){
        return;
    }
    if(!realPosition.cInterpDiscrete(position, this->snapInterpRate)){
        recalculate();    
    }    

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

void nite::PanelUI::setBackgroundColor(const nite::Color &color){
    if(baseColor == color){
        return;
    }
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
