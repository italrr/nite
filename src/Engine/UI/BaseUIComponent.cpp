#include "BaseUIComponent.hpp"
#include "../Shapes.hpp"
#include "../Input.hpp"
#include <cmath>
#include "../Shapes.hpp"
#include "../Console.hpp"
#include "UI.hpp"

bool __nite_showUISpacingFrames = false;
// static nite::Console::CreateProxy showUIFrames("cl_uiframes", nite::Console::ProxyType::Bool, sizeof(bool), &__nite_showUISpacingFrames);

static String getUniqueId(){
    static UInt64 id = nite::randomInt(1500, 8500);
    return nite::hashString(nite::toStr(nite::getTicks() + ++id));
}

nite::NavUI::NavUI(){
    current = 0;
    enable = false;
    index = 0;
    split = 0;
    cursor = 0.0f;
    a = nite::Color(0.89f, 0.87f, 0.15f, 1.0f);
    b = nite::Color(0.83f, 0.79f, 0.10f, 1.0f);
    color = a;
    expInc = 0.0f;
    colorFlip = false;
}

void nite::NavUI::reset(){
    current = 0;
    enable = false;
    index = 0;
    split = 0;
    cursor = 0.0f;
}

void nite::NavUI::update(BaseUIComponent *comp){
    if(comp == NULL){
        return;
    }

    auto getSelectedChild = [&](){
        for(int i = 0; i < comp->children.size(); ++i){
            if(comp->children[i]->nav.index == current){
                return comp->children[i];
            }
        }
        return Shared<nite::BaseUIComponent>(NULL);
    };

    auto solveAutoScroll = [&](){
        if(!comp->allowOverflow){
            return;
        }
        auto child = getSelectedChild();
        if(child.get() == NULL){
            return;
        }
        auto space = comp->computeSize();
        auto area = child->computeSize();
        auto p = child->position + comp->scrollOffset;
        if( p.x + area.x * 0.5f <= 0.0f || p.y + area.y * 0.5f <= 0.0f || 
            p.x - area.x * 0.5f >= space.x || p.y - area.y * 0.5f >= space.y ||
            p.x - area.x * 0.5f < 0.0f || p.y - area.y * 0.5f < 0.0f || 
            p.x + area.x * 0.5f > space.x || p.y + area.y * 0.5f > space.y){
            // TODO: this is going to change to accodomate vinline (instead of left to right, top to bottom)
            // see commented line below
            comp->scrollOffset.y = -(child->position.y - area.y * 0.5f) + area.y;
            //comp->scrollOffset.x = -(child->position.x - area.x * 0.5f);
            comp->recalculate();
        }        
    };

	auto lrpUpd = color.cInterpDiscrete(colorFlip ? a : b, colorFlip ? 0.34f : 0.11f);
    nite::lerpDiscrete(expInc, colorFlip ? 100 : 0, colorFlip ? 0.34f : 0.11f);

	if(comp->visible){
		for(auto &key : comp->keyListeners){
			if(nite::keyboardPressed(nite::translateKey(key.first))){
				key.second(Shared<nite::ListenerInfo>(new nite::ListenerInfo()), comp);
			}
		}		
	}

	if(enable && lrpUpd){
		colorFlip = !colorFlip;
		comp->recalculate();
	}

	if(enable && !lrpUpd){
		comp->recalculate();	
	}

	if(enable && nite::keyboardPressed(comp->onClickAnalogueKey)){
		for(int i = 0; i < comp->children.size(); ++i){
			if(comp->children[i]->nav.index == current){
				comp->children[i]->onClick();
			}
		}
	}

	if(enable && nite::keyboardPressed(nite::keyRIGHT)){
		if(comp->layout->xorient && !comp->layout->yorient || !comp->layout->xorient && comp->layout->yorient){
			current = current+1 >= comp->children.size() ? current : current + 1;
		}else
		if(comp->layout->xorient && comp->layout->yorient && split != 0){
			current = (current + 1) % split == 0 || (current + 1) > comp->children.size()-1 ? current : current + 1;
		}		
        solveAutoScroll();
		comp->recalculate();
	}

	if(enable && nite::keyboardPressed(nite::keyLEFT)){
		if(comp->layout->xorient && !comp->layout->yorient || !comp->layout->xorient && comp->layout->yorient){
			current = current-1 < 0 ? current : current - 1;
		}else
		if(comp->layout->xorient && comp->layout->yorient && split != 0){
			--current;
			if(current < 0 || (current + 1) % split == 0){
				++current;
			}
		}		
        solveAutoScroll();
		comp->recalculate();
	}

	if(enable && nite::keyboardPressed(nite::keyDOWN)){
		if(comp->layout->xorient && !comp->layout->yorient || !comp->layout->xorient && comp->layout->yorient){
			current = (current + 1) >= comp->children.size() ? current : current + 1;
		}else
		if(comp->layout->xorient && comp->layout->yorient && split != 0){
			current += split;
			if(current > comp->children.size()-1){
				current -= split;
			}
		}		
        solveAutoScroll();
		comp->recalculate();
	}	


	if(enable && nite::keyboardPressed(nite::keyUP)){
		if(comp->layout->xorient && !comp->layout->yorient || !comp->layout->xorient && comp->layout->yorient){
			current = current-1 < 0 ? current : current - 1;
		}else
		if(comp->layout->xorient && comp->layout->yorient && split != 0){
			current -= split;
			if(current < 0){
				current += split;
			}
		}	      
        solveAutoScroll();	
		comp->recalculate();
	}
}

nite::PoliVec2::PoliVec2(){
    useAbs = true;
    abs.set(32.0f);
}

nite::PoliVec2::PoliVec2(const String &w, const String &h){
    set(w, h);
}

nite::PoliVec2::PoliVec2(const nite::Vec2 &size){
    set(size);
}

void nite::PoliVec2::set(float x, float y){
    this->useAbs = true;
    this->abs.set(x, y);
}

nite::PoliVec2::PoliVec2(float x, float y){
    set(x, y);
}

void nite::PoliVec2::set(const String &w, const String &h){
    this->useAbs = false;
    String wi = nite::strRemoveSpaces(w);
    String hi = nite::strRemoveSpaces(h);
    if(wi[wi.length()-1] == '%'){
        wi.erase(wi.length()-1);
    }
    if(hi[hi.length()-1] == '%'){
        hi.erase(hi.length()-1);
    }    
    this->rel.x = nite::toFloat(wi) / 100.0f;
    this->rel.y = nite::toFloat(hi) / 100.0f;
}

void nite::PoliVec2::set(const nite::Vec2 &size){
    set(size.x, size.y);
}

float nite::PoliVec2::x(float rw){
    return useAbs ? abs.x : rel.x * rw;
}

float nite::PoliVec2::y(float rh){
    return useAbs ? abs.y : rel.y * rh;
}

nite::BaseUIComponent::BaseUIComponent(){
    padding.set(0.0f);
    margin.set(0.0f);
    size.set(16.0f);
    position.set(0.0f);
    baseColor.set(0.0f, 0.0f, 0.0f, 1.0f);
    zdepth = 0;
    useRelSizeX = false;
    useRelSizeY = false;
    type = "base";
    relSize.set(0.0f);
    flex = 0.0f;
    visible = true;
    allowOverflow = false;
    scrollX = false;
    scrollY = false;
    scrollOffset.set(0.0f);
    realPosition.set(0.0f);
    scrollOffsetTrans.set(0.0f);
    solid = false;
    onClickAnalogueKey = nite::keyZ;
    fillUpType = false;
    layout = std::shared_ptr<nite::LayoutSystemUI>(new nite::Layout::HBox());
    headComponent = NULL;
    renderOnTarget = nite::RenderTargetUI;
    isHovered = false;
    uninteract = false;
    useShader = false;
    literalId = getUniqueId();
    snapInterpRate = componentTranslationSpeed;
}

void nite::BaseUIComponent::updateListeners(){
    if(uninteract) return;
    nite::Vec2 rp(position + relativePosition);
    auto mp = nite::mousePosition();
    auto cps = computeSize();
    nite::Vec2 p = rp - cps * 0.5f + margin * 0.5f;
    // HOVER
    if(nite::isPointInRegion(mp, p, p + cps - margin)){
        isHovered = true;
        onHover();
    }else
    if(isHovered){
        isHovered = false;
        onUnhover();
    }
    // CLICK
    if(nite::isPointInRegion(mp, p, p + cps - margin) && nite::mousePressed(nite::butLEFT)){
        onClick();
    }
    if(__nite_showUISpacingFrames){
        nite::setRenderTarget(nite::RenderTargetUI);
        if(isHovered){
            nite::setColor(1.0f, 0.0f, 1.0f, 0.4f);
            nite::Draw::Rectangle(p.x, p.y, cps.x - margin.x, cps.y - margin.y, true);      
        }
        nite::setColor(1.0f, 0.0f, 1.0f, 1.0f);
        nite::Draw::Rectangle(p.x, p.y, cps.x - margin.x, cps.y - margin.y, false);
    }
  // if(auto *comp = dynamic_cast<nite::PanelUI*>(this)){
  // }
}

bool nite::BaseUIComponent::add(std::shared_ptr<nite::BaseUIComponent> component){
    if(component.get() == NULL){
        nite::print("attempted to add NULL BaseUIComponent in a Component.");
        return false;
    }
    for(int i = 0; i < children.size(); ++i){
        if(children[i].get() == component.get()){
            nite::print("attempted to add a BaseUIComponent twice in a Component.");
            return false;
        }
    }
    children.push_back(component);
    component->onCreate();
    component->preinit();
    component->headComponent = this;
    recalculate();
    return true;
}

std::shared_ptr<nite::BaseUIComponent> nite::BaseUIComponent::add(Jzon::Node &node){
    auto top = getTopHeadComponent(); // shouldn't be null
    auto cmp = nite::UI::build(node, top->styles); // in this case we're overlooking for listeners and idsLookUp. TODO
    if(cmp.get() != NULL){
        this->add(cmp);
    }
    return cmp;
}

bool nite::BaseUIComponent::remove(int id){
    for(int i = 0; i < children.size(); ++i){
        if(children[i]->id == id){
            children[i]->onDestroy();
            children[i]->headComponent = NULL;
            children.erase(children.begin() + i);
            recalculate();
            return true;
        }
    }
    return false;
}

void nite::BaseUIComponent::fixNavIndexes(){
    for(int i = 0; i < children.size(); ++i){
        children[i]->nav.index = i;
    }
}

void nite::BaseUIComponent::destroy(){
    nite::UI::remove(this); 
}  

void nite::Layout::VBox::recalculate(BaseUIComponent &head){
    auto offset = head.padding * nite::Vec2(0.5f);
    float leftVertSpace = head.size.y - head.padding.y;
    nite::Vec2 spaceLeft(head.size - head.padding);  
    nite::Vec2 totalSpace(head.size - head.padding);
    // let's calculate free space and flexes
    float  vcursor = 0.0f;
    float equalDiv = 0.0f;
    int haveFlex = 0;
    float zeroFlexSizes = 0.0f;  
    for(int i = 0; i < head.children.size(); ++i){
        auto child = head.children[i];      
        if(!child->isVisible()){
            continue;
        }        
        equalDiv += child->flex;
        if(child->flex > 0){
            ++haveFlex;
        }
        if(child->flex <= 0){
            auto cps = child->computeSize();      
            zeroFlexSizes += cps.y;
        }
    }
    zeroFlexSizes = zeroFlexSizes > 0.0f ? totalSpace.y - zeroFlexSizes : totalSpace.y;
    // apply size
    for(int i = 0; i < head.children.size(); ++i){
        auto child = head.children[i];
        if(!child->isVisible()){
            continue;
        }
        float ownDiv = child->flex / equalDiv;
        float ownDivSize = ownDiv * zeroFlexSizes;
        bool applyZeroFlex = ownDivSize > 0.0f;
        if(child->useRelSizeX || child->useRelSizeY){
            nite::Vec2 ap;
            ap.x = child->useRelSizeX ? totalSpace.x * child->relSize.x : (child->size.x == -1.0f ? totalSpace.x : child->size.x);
            ap.y = child->useRelSizeY ? totalSpace.y * child->relSize.y : (child->size.y == -1.0f ? totalSpace.y : child->size.y);
            child->accommodate(ap.x, ap.y);
        }else   
        if(child->fillUpType){
            child->accommodate(totalSpace.x, child->flex > 0.0f ? ownDivSize : totalSpace.y);
        }
        auto cps = child->computeSize();
        if(cps.y > leftVertSpace){
            child->position.set(-1.0f, -1.0f);
        }
        child->position.set(nite::Vec2(0.0f, vcursor) + offset + cps * nite::Vec2(0.5f));
        float hamnt = applyZeroFlex ? ownDivSize : cps.y;
        vcursor += hamnt;
        leftVertSpace -= hamnt;
    }
}

void nite::Layout::HBox::recalculate(BaseUIComponent &head){
    auto offset = head.padding * nite::Vec2(0.5f);
    float leftHorSpace = head.size.x - head.padding.x;
    nite::Vec2 spaceLeft(head.size - head.padding);  
    nite::Vec2 totalSpace(head.size - head.padding);
    float  hcursor = 0.0f;
    float equalDiv = 0.0f;
    int haveFlex = 0;
    float zeroFlexSizes = 0.0f;  
    for(int i = 0; i < head.children.size(); ++i){
        auto child = head.children[i];      
        if(!child->isVisible()){
            continue;
        }        
        equalDiv += child->flex;
        if(child->flex > 0){
            ++haveFlex;
        }
        if(child->flex <= 0){
            auto cps = child->computeSize();      
            zeroFlexSizes += cps.x;
        }    
    }
    zeroFlexSizes = zeroFlexSizes > 0.0f ? totalSpace.x - zeroFlexSizes : totalSpace.x;
    for(int i = 0; i < head.children.size(); ++i){
        auto child = head.children[i];
        if(!child->isVisible()){
            continue;
        }        
        float ownDiv = child->flex / equalDiv;
        float ownDivSize = ownDiv * zeroFlexSizes;
        bool applyZeroFlex = ownDivSize > 0.0f;
        if(child->useRelSizeX || child->useRelSizeY){
            nite::Vec2 ap;
            ap.x = child->useRelSizeX ? totalSpace.x * child->relSize.x : (child->size.x == -1.0f ? totalSpace.x : child->size.x);
            ap.y = child->useRelSizeY ? totalSpace.y * child->relSize.y : (child->size.y == -1.0f ? totalSpace.y : child->size.y);
            child->accommodate(ap.x, ap.y);
        }else         
        if(child->fillUpType){
            child->accommodate(child->flex > 0.0f ? ownDivSize : totalSpace.x, totalSpace.y);
        }
        auto cps = child->computeSize();
        if(cps.x > leftHorSpace){
            child->position.set(-1.0f, -1.0f);
        }
        child->position.set(nite::Vec2(hcursor, 0.0f) + offset + cps * nite::Vec2(0.5f));
        float hamnt = applyZeroFlex ? ownDivSize : cps.x;
        hcursor += hamnt;
        leftHorSpace -= hamnt;
    }
}

void nite::Layout::Inline::recalculate(BaseUIComponent &head){
    auto offset = head.padding * nite::Vec2(0.5f);
    nite::Vec2 cursor(0.0f);
    nite::Vec2 spaceLeft(head.size - head.padding);
    nite::Vec2 totalSpace(head.size - head.padding);
    float lastLineHeight = 0.0f;
    for(int i = 0; i < head.children.size(); ++i){
        auto child = head.children[i];
        if(!child->isVisible()){
            continue;
        }        
        if(child->useRelSizeX || child->useRelSizeY){
            nite::Vec2 ap;
            ap.x = child->useRelSizeX ? totalSpace.x * child->relSize.x : (child->size.x == -1.0f ? totalSpace.x : child->size.x);
            ap.y = child->useRelSizeY ? totalSpace.y * child->relSize.y : (child->size.y == -1.0f ? totalSpace.y : child->size.y);
            child->accommodate(ap.x, ap.y);
        }else       
        if(child->fillUpType){
            child->accommodate(totalSpace.x, totalSpace.y);
        }
        auto cps = child->computeSize();
        if(cursor.x > 0.0f && cps.x > spaceLeft.x){
            spaceLeft.x = head.size.x - head.padding.x;
            cursor.x = 0.0f;
            cursor.y += lastLineHeight;
            lastLineHeight = 0.0f;
        }
        if(cps.y > spaceLeft.y){
            child->position.set(-1.0f, -1.0f);
        }
        child->position.set(cursor + offset + cps * nite::Vec2(0.5f));
        spaceLeft.x -= cps.x;
        spaceLeft.y -= cps.y;
        cursor.x += cps.x;
        lastLineHeight = std::max(lastLineHeight, cps.y);
    }
}