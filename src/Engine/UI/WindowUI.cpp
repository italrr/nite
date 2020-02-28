#include "WindowUI.hpp"
#include "../Input.hpp"
#include "../Shapes.hpp"

extern bool __nite_showUISpacingFrames;

nite::WindowUI::WindowUI(const String &title){
	this->title = title;
	defaultInit();
}

nite::WindowUI::WindowUI(){
	this->title = "Window"; 
	defaultInit();
}

nite::Vec2 nite::WindowUI::computeSize(){
	return size;
}

void nite::WindowUI::setOnRerenderListener(nite::ListenerLambda listener){
	this->onRerender = listener;
}

void nite::WindowUI::updateListeners(){
	if(uninteract) return;  
	nite::Vec2 rp(position);
	auto mp = nite::mousePosition();
	auto cps = computeSize();
	nite::Vec2 p = rp;

	// HOVER
	if(nite::isPointInRegion(mp, p, p + cps - margin)){
		onHover();
	}
	// CLICK
	if(nite::isPointInRegion(mp, p, p + cps - margin) && nite::mousePressed(nite::butLEFT)){
		onClick();
	}
	if(__nite_showUISpacingFrames){
		nite::setRenderTarget(nite::RenderTargetUI);
		nite::setColor(1.0f, 0.0f, 1.0f, 1.0f);
		nite::Draw::Rectangle(p.x, p.y, size.x, size.y, false);
	}
	// if(auto *comp = dynamic_cast<nite::PanelUI*>(this)){
	// }
}    

void nite::WindowUI::defaultInit(){
	componentName = "Window";   
	fillUpType = false;
	resizeGrab = false;
	grabbed = false;  
	size.set(nite::Vec2(300, 500) * getGeneralScale());
	unmovable = false;
	resizeable = true;
	borderThickness = 8.0;
	enableTitle = true;
	baseColor.set(0.0f, 0.0f, 0.0f, 1.0f);
	leftBorderColor.set(0.88f, 0.1f, 0.1f, 1.0f);
	titleColor.set(1.0f, 1.0f, 1.0f, 1.0f);
	toDestroy = false;
	generalAlpha = 100.0;
	this->onRerender = [](const Shared<nite::ListenerInfo> &info, nite::BaseUIComponent &component){
		return;
	};	
}

void nite::WindowUI::setBackgroundImage(const nite::Texture &bgi){
	uiBackgroundImage = bgi;
}

nite::Texture nite::WindowUI::getBackgroundImage(){
	return uiBackgroundImage;	
}

void nite::WindowUI::setPosition(const nite::Vec2 &pos){
	position.set(pos);
	origPosition.set(position + nite::Vec2(0.0f, 32.0f));
}

nite::Color nite::WindowUI::getBackgroundColor(){
	return baseColor;
}

nite::Color nite::WindowUI::getBorderColor(){
	return leftBorderColor;
}

nite::Color nite::WindowUI::getTitleColor(){
	return titleColor;
}

String nite::WindowUI::getTitle(){
	return title;
}

void nite::WindowUI::setBorderColor(const nite::Color &color){
	this->leftBorderColor.set(color);
	recalculate();  
}

void nite::WindowUI::setTitleColor(const nite::Color &color){
	this->titleColor.set(color);
	recalculate();  
}

void nite::WindowUI::setBackgroundColor(const nite::Color &color){
	this->baseColor.set(color);
	recalculate();  
}

void nite::WindowUI::setTitle(const String &title){
	this->title = title;
	recalculate();
}

void nite::WindowUI::setSize(const nite::Vec2 &size){
	this->size.set(size);
	recalculate();
}

void nite::WindowUI::setShowTitle(bool v){
	enableTitle = v;
	recalculate();
}

void nite::WindowUI::setBorderThickness(float tn){
	borderThickness = tn;
	recalculate();  
}

void nite::WindowUI::close(){
	setInteractive(false);
	toDestroy = true;
}

void nite::WindowUI::rerenderDecoration(){
	decoration.begin();  
	// Render Base
	if(uiBackgroundImage.isLoaded()){
		auto *base = uiBackgroundImage.draw(0, 0, size.x, size.y, 0.0f, 0.0f, 0.0f);
		if(base != NULL){
			base->repeat = true;
		}
	}else{
		// TODO: check this later. Border and background should be rendered separate 
		auto *base = uiBasicTexture.draw(0, 0, size.x, size.y, 0.0f, 0.0f, 0.0f);
		if(base != NULL){
			base->color = leftBorderColor;
			nite::Uniform uni;
			uni.add("p_size", size);
			uni.add("p_bcolor", baseColor);
			uni.add("p_lbcolor", borderColor);
			uni.add("p_salpha", leftBorderColor.a);
			uni.add("p_alpha", baseColor.a);
			uni.add("p_hborder", nite::Vec2(borderThickness * getGeneralScale()));
			uni.add("p_vborder", nite::Vec2(enableTitle ? headerHeight : borderThickness, borderThickness * getGeneralScale()));     
			uni.add("p_blurthickness", 2.0f * (1.0f / ((size.avg() * getGeneralScale()) / nite::Vec2(nite::Vec2(300, 500)*getGeneralScale()).avg()))); // 300x500 was the original design size
			base->apply(uiShader, uni);
		}
	}
	// Render corners
	if(borderCorner.isLoaded()){
		nite::setColor(leftBorderColor);
		nite::Vec2 sz = borderCorner.getSize();
		// UPPER LEFT
		auto *ul = borderCorner.draw(0, 0, borderCorner.getWidth(), borderCorner.getHeight(), 0.0f, 0.0f, 0.0f);
		if(ul != NULL){
			ul->smooth = true;
		}
		// UPPER RIGHT
		auto *ur = borderCorner.draw(size.x, 0, -borderCorner.getWidth(), borderCorner.getHeight(), 0.0f, 0.0f, 0.0f);
		if(ur != NULL){
			ul->smooth = true;
		}
		// LOWER LEFT
		auto *ll = borderCorner.draw(0, size.y, borderCorner.getWidth(), -borderCorner.getHeight(), 0.0f, 0.0f, 0.0f);
		if(ll != NULL){
			ll->smooth = true;
		}
		// LOWER RIGHT
		auto *lr = borderCorner.draw(size.x, size.y, -borderCorner.getWidth(), -borderCorner.getHeight(), 0.0f, 0.0f, 0.0f);
		if(lr != NULL){
			lr->smooth = true;
		}
	}

	// Render Title
	if(enableTitle){
		nite::setColor(titleColor);
		auto *titleRef = basicFont.draw(title, nite::Vec2(size.x * 0.5f, headerHeight * 0.65f), nite::Vec2(0.5f), 0.65f, 0.0f);
		if(titleRef != NULL){
			titleRef->setShadow(nite::Color(0.0f, 0.0f, 0.0f, 1.0f), nite::Vec2(1.5f));
		}
	}
	decoration.end();
	decoration.flush();	
}

void nite::WindowUI::rerender(){
	if(!toRerender) return;
	rerenderDecoration();
	batch.begin();  
	// Render Children
	if(this->visible){
		for(int i = 0; i < children.size(); ++i){
			if(children[i]->position.x < 0 || children[i]->position.y < 0 || !children[i]->visible){
				continue;
			}
			children[i]->beforeRender();
			children[i]->render();
			children[i]->afterRender();
		}		
	}
	// auto offset = padding * nite::Vec2(0.5f);
	// static nite::Texture tex("data/sprite/empty.png");
	// nite::setRenderTarget(nite::RenderTargetUI);
	// nite::setColor(1.0f, 0.0f, 0.0f, 1.0f);
	// nite::setDepth(nite::DepthTop);
	// tex.draw(offset.x, offset.y, size.x - padding.x, size.y - padding.y, 0.0f, 0.0f, 0.0f);
	batch.end();
	batch.flush();
	toRerender = false;
	this->onRerender(Shared<nite::ListenerInfo>(new nite::ListenerInfo()), *this);
}


void nite::WindowUI::recalculate(){
	batch.init(size.x, size.y);
	decoration.init(size.x, size.y);
	padding.set(52.0f * getGeneralScale(), (enableTitle ? headerHeight * 2.0f + 24.0f : 52.0f) * getGeneralScale());
	layout->recalculate(*this);
	toRerender = true;
}

void nite::WindowUI::setCornerPattern(const nite::Texture &tex){
	borderCorner = tex;
}

void nite::WindowUI::onCreate(){
	basicFont.load(defaultFontUI, defaultFontRatio * 16 * getGeneralScale());
	uiBasicTexture.load("data/sprite/empty.png");
	uiShader.load("data/shaders/basic_ui_background_f.glsl", "data/shaders/basic_ui_background_v.glsl");
	headerHeight = basicFont.getHeight() * 0.5f * 2.0f;  
	grabbed = false;
	toRerender = true;
	resizeGrab = false;  
	recalculate();
}

void nite::WindowUI::update(){
//   origPosition.set(toDestroy ? (position - nite::Vec2(0.0f, -16.0f)) : position);      
	origPosition.lerp(toDestroy ? (position - nite::Vec2(0.0f, -16.0f)) : position, 0.15f);  
	
	if(!borderColor.lerpDiscrete(grabbed ? leftBorderColor * 1.5f : leftBorderColor, 0.08f)){
		// recalculate();
		rerenderDecoration();
	}
	
	if(toDestroy){
		if(nite::lerpDiscrete(generalAlpha, 0.0f, 0.15f)){
			destroy();
			return;
		}
	}
	// Drag
	if(!unmovable && nite::mousePressed(nite::butLEFT) && nite::isPointInRegion(nite::mousePosition(), position + nite::Vec2(0.0f, headerHeight * 0.3), position + nite::Vec2(size.x, headerHeight))){
		grabbed = true;
		grabbedPoint.set(nite::mousePosition() - position);
		recalculate();			
	}
	if(nite::mouseReleased(nite::butLEFT) && grabbed){
		grabbed = false;
		recalculate();			
	}
	if(grabbed){
		origPosition.set(nite::mousePosition() - grabbedPoint);
		position.set(origPosition);
	}
	// Resize

	bool onTopRegion = nite::isPointInRegion(nite::mousePosition(), position, position + nite::Vec2(size.x, size.y * 0.03f));  
	bool onLeftRegion = nite::isPointInRegion(nite::mousePosition(), position, position + nite::Vec2(size.x * 0.05f, size.y));
	bool onRightRegion = nite::isPointInRegion(nite::mousePosition(), position + nite::Vec2(size.x * 0.95f, 0.0f), position + nite::Vec2(size.x * 0.95f, 0.0f) + nite::Vec2(size.x * 0.05f, size.y));  
	bool onBottomRegion = nite::isPointInRegion(nite::mousePosition(), position + nite::Vec2(0.0f, size.y * 0.95f), position + nite::Vec2(0.0f, size.y * 0.95f) + nite::Vec2(size.x, size.y * 0.05f));  
	if(resizeable && nite::mousePressed(nite::butLEFT) && !grabbed && !resizeGrab && (onLeftRegion || onRightRegion || onTopRegion || onBottomRegion) ){
		resizeGrab = true;
		grabbedPoint.set(nite::mousePosition() - position);    
		if(onLeftRegion){
			resizeOrientation = nite::WindowResizeOrientation::Left;
		}else
		if(onRightRegion){
			resizeOrientation = nite::WindowResizeOrientation::Right;
		}else
		if(onTopRegion){
			resizeOrientation = nite::WindowResizeOrientation::Up;
		}else
		if(onBottomRegion){
			resizeOrientation = nite::WindowResizeOrientation::Down;
		}
		resizeGrabPoint.set(nite::mousePosition());
		beforeGrabSize.set(size);
	}

	if(resizeGrab){
		// DOWNWARDs
		if(resizeOrientation == nite::WindowResizeOrientation::Down){
			auto currentMouse = nite::mousePosition();
			//position.x = nite::mousePosition().x - grabbedPoint.x;
			float dist = currentMouse.y - resizeGrabPoint.y;
			if(beforeGrabSize.y + dist > 150.0f){
				size.set(beforeGrabSize.x, beforeGrabSize.y + dist);
			}
			recalculate();
			rerender(); 
		}    
		// UPWARDS
		if(resizeOrientation == nite::WindowResizeOrientation::Up){
			auto currentMouse = nite::mousePosition();
			position.y = nite::mousePosition().y - grabbedPoint.y;
			origPosition.y = position.y;
			float dist = resizeGrabPoint.y - currentMouse.y;
			if(beforeGrabSize.y + dist > 250.0f){
				size.set(beforeGrabSize.x, beforeGrabSize.y + dist);
			}
			recalculate();
			rerender();      
		}      
		// TO LEFT
		if(resizeOrientation == nite::WindowResizeOrientation::Left){
			auto currentMouse = nite::mousePosition();
			position.x = nite::mousePosition().x - grabbedPoint.x;
			origPosition.x = position.x;
			float dist = resizeGrabPoint.x - currentMouse.x;
			if(beforeGrabSize.x + dist > 150.0f){
				size.set(beforeGrabSize.x + dist, beforeGrabSize.y);
			}
			recalculate();
			rerender();      
		}   
		// TO RIGHT
		if(resizeOrientation == nite::WindowResizeOrientation::Right){
			auto currentMouse = nite::mousePosition();
			//position.x = nite::mousePosition().x - grabbedPoint.x;
			float dist = currentMouse.x - resizeGrabPoint.x;
			if(beforeGrabSize.x + dist > 150.0f){
				size.set(beforeGrabSize.x + dist, beforeGrabSize.y);
			}
			recalculate();
			rerender(); 
		}      
	}
	if(resizeGrab && nite::mouseReleased(nite::butLEFT)){
		resizeGrab = false;
		beforeGrabSize.set(size);
	}

	for(int i = 0; i < children.size(); ++i){
		if(!uninteract){
			children[i]->updateRelativePosition(position);
			children[i]->updateListeners();
		}
		children[i]->beforeUpdate();
		children[i]->update();
		children[i]->afterUpdate();
	}
}

void nite::WindowUI::render(){
	rerender();

	// Render batch
	nite::setRenderTarget(renderOnTarget);
	nite::setDepth(nite::DepthMiddle);
	float d = 1.0f; // 1.0f - nite::distance(origPosition, position) / 32.0f;
	nite::setColor(1.0f, 1.0f, 1.0f, d * (generalAlpha / 100.0f));
	// auto *ref = decoration.draw(origPosition.x, origPosition.y);
	// if(ref != NULL){
	// 	ref->color.set(0.0f, 0.0f, 0.0f, 0.1f);
	// }
	decoration.draw(origPosition.x, origPosition.y);
	batch.draw(origPosition.x, origPosition.y);
}