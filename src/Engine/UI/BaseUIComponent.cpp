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

nite::BaseUIComponent::BaseUIComponent(){
    padding.set(0.0f);
    margin.set(0.0f);
    size.set(16.0f);
    position.set(0.0f);
    baseColor.set(0.0f, 0.0f, 0.0f, 1.0f);
    zdepth = 0;
    flex = 0.0f;
    visible = true;
    solid = false;
    fillUpType = false;
    layout = std::shared_ptr<nite::LayoutSystemUI>(new nite::Layout::Inline());
    headComponent = NULL;
    renderOnTarget = nite::RenderTargetUI;
    isHovered = false;
    uninteract = false;
    literalId = getUniqueId();
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

void nite::BaseUIComponent::add(std::shared_ptr<nite::BaseUIComponent> component){
  if(component.get() == NULL){
    nite::print("Attempt to add NULL nite::BaseUIComponent in a Component.");
    return;
  }
  for(int i = 0; i < children.size(); ++i){
    if(children[i].get() == component.get()){
      nite::print("Attempt to add a nite::BaseUIComponent twice in a Component.");
      return;
    }
  }
  children.push_back(component);
  component->onCreate();
  component->preinit();
  component->headComponent = this;
  recalculate();
}

void nite::BaseUIComponent::destroy(){
  nite::UI::remove(this);
}  

void nite::Layout::VBox::recalculate(BaseUIComponent &head){
  auto offset = head.padding * nite::Vec2(0.5f);
  float leftVertSpace = head.size.y - head.padding.y;
  nite::Vec2 spaceLeft(head.size - head.padding);  
  nite::Vec2 totalSpace(head.size - head.padding);
  float  vcursor = 0.0f;
  float equalDiv = 0.0f;
  int haveFlex = 0;
  float zeroFlexSizes = 0.0f;  
  for(int i = 0; i < head.children.size(); ++i){
    auto child = head.children[i];      
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

  for(int i = 0; i < head.children.size(); ++i){
    auto child = head.children[i];
    float ownDiv = child->flex / equalDiv;
    float ownDivSize = ownDiv * zeroFlexSizes;
    bool applyZeroFlex = ownDivSize > 0.0f;
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
    float ownDiv = child->flex / equalDiv;
    float ownDivSize = ownDiv * zeroFlexSizes;
    bool applyZeroFlex = ownDivSize > 0.0f;
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