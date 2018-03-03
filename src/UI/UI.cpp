#include "UI.hpp"
#include "../Engine/Tools/Tools.hpp"

void nite::UIMaster::add(std::shared_ptr<BaseUIComponent> comp){
  if(comp == NULL){
    return;
  }
  for(int i = 0; i < components.size(); ++i){
    if(components[i].get() == comp.get()){
      nite::print("Attempt to add a nite::BaseUIComponent twice in UI Master.");
      return;
    }
  }
  components.push_back(comp);
  comp->currentMaster = this;
  comp->onCreate();
  comp->preinit();
}

void nite::UIMaster::render(){
  for(int i = 0; i < components.size(); ++i){
    components[i]->beforeRender();
    components[i]->render();
    components[i]->afterRender();
  }
}

void nite::UIMaster::remove(nite::BaseUIComponent *comp){
  removeQueue.push_back(comp);
}

struct JsonSource {
  String path;
  String hash;
  UInt64 lastTick;
  Shared<nite::BaseUIComponent> ui;
  Dict<String, nite::ListenerLambda> listeners;
  nite::ListenerLambda getListener(const String &id){
    auto dummy = [](const Shared<nite::ListenerInfo> &info, const nite::BaseUIComponent &component){
      return;
    };
    return listeners.find(id) != listeners.end() ? listeners[id] : dummy;
  }
};

static Vector<JsonSource> sources;

static float expression(const String &input, Shared<nite::BaseUIComponent> &component){
  if(!(input.length() > 3 && input[0] == '!' && input[1] == '!' && input[2] == ' ')) return 0.0f;
  auto element = nite::split(input, ' ');
  auto getGlobalToken = [&](const String &token){
    if(nite::toLower(token) == "gheight"){
      return (float)nite::getHeight();
    }else
    if(nite::toLower(token) == "gwidth"){
      return (float)nite::getWidth();
    }else{
      return 0.0f;
    }
  };
  auto getLocalToken = [&](const String &token, Shared<nite::BaseUIComponent> &component){
    String tok = nite::toLower(token);
    if(tok == "width"){
      return component->computeSize().x;
    }
    if(tok == "height"){
      return component->computeSize().y;
    }
  };
  auto parseToken = [&](const String &token){
    if(token.length() == 0) return 0.0f;
    switch(token[0]){
      // Global
      case '#':
        return getGlobalToken(token.substr(1, token.length()-1));
      case '$':
      // Local
        return getLocalToken(token.substr(1, token.length()-1), component);
      default:
        auto toktyp = String("")+token[0];
        nite::print("Failed to parse expression from UI JSON. Token: '"+toktyp+"'");
        return 0.0f;
    }
  };
  auto parseFragment = [&](const String &c){
    // NUMBER
    if(nite::isNumber(c)){
      return (float)nite::toFloat(c);
    }else{
    // TOKEN
      return (float)parseToken(c);
    }
  };
  float result = 0.0f;
  auto solve = [&](const String &op, const String &arg1, const String &arg2){
    float _solv = 0.0f;
    //substraction
    if(op == "-"){
      _solv = parseFragment(arg1) - parseFragment(arg2);
    }else
    if(op == "+"){
      _solv = parseFragment(arg1) + parseFragment(arg2);
    }else
    if(op == "*"){
      _solv = parseFragment(arg1) * parseFragment(arg2);
    }else
    if(op == "/"){
      _solv = parseFragment(arg1) / parseFragment(arg2);
    }else{
      nite::print("Failed to parse expression from UI JSON: Unknown operator '"+op+"': Syntax Error");
    }            
    return _solv;
  };
  auto isOp = [&](const String &op){
    return op == "-" || op == "+" || op == "/" || op == "*";
  };
  // remove !!
  element.erase(element.begin());
  for(int i = 0; i < element.size(); ++i){
    auto &c = element[i];
    auto cop = isOp(c);
    if(cop && i > 0 && i < element.size()-1){
      auto pre = element[i - 1];
      auto next = element[i + 1];
      element[i + 1] = nite::toStr(solve(c, pre, next));
      element.erase(element.begin() + i - 1);
      element.erase(element.begin() + i - 1);
      i = -1;
    }else
    if(cop){
      nite::print("Failed to parse expression from UI JSON: '"+input+"': Syntax Error");
      return 0.0f;
    }
  }
  return nite::toFloat(element[0]);
}

static float _parseGenericFloat(const Jzon::Node &node, const String &name, Shared<nite::BaseUIComponent> &component){
  if(!node.has(name)){
    return 0.0f;
  }
  if(node.get(name).isNumber()){
    return node.get(name).toFloat();
  }else{
    return expression(node.get(name).toString(), component);
  }
}

static int _parseGenericInt(const Jzon::Node &node, const String &name, Shared<nite::BaseUIComponent> &component){
  if(!node.has(name)){
    return 0.0f;
  }
  if(node.get(name).isNumber()){
    return node.get(name).toFloat();
  }else{
    return (int)expression(node.get(name).toString(), component);
  }
}

/*** [ PARSE METHOD ] 
    WIDTH & HEIGHT
***/
static nite::Vec2 _parseSize(const Jzon::Node &node, const nite::Vec2 &contingency, Shared<nite::BaseUIComponent> &component){
  float width = _parseGenericFloat(node, "width", component);
  float height = _parseGenericFloat(node, "height", component);
  return nite::Vec2(width == 0.0f ? contingency.x : width, height == 0.0f ? contingency.y : height);
}

static nite::Vec2 _parsePosition(const Jzon::Node &node, const nite::Vec2 &contingency, Shared<nite::BaseUIComponent> &component){
  float width = _parseGenericFloat(node, "x", component);
  float height = _parseGenericFloat(node, "y", component);
  return nite::Vec2(width == 0.0f ? contingency.x : width, height == 0.0f ? contingency.y : height);
}

/*** [ PARSE METHOD ] 
    LAYOUT
***/
static Shared<nite::LayoutSystemUI> _parseLayout(const Jzon::Node &node, Shared<nite::BaseUIComponent> &component){
  String layout = node.get("layout").toString();
  if(layout == "vbox" || layout == ""){
    return Shared<nite::LayoutSystemUI>(new nite::Layout::VBox());
  }else
  if(layout == "hbox"){  
    return Shared<nite::LayoutSystemUI>(new nite::Layout::HBox());    
  }else
  if(layout == "inline"){  
    return Shared<nite::LayoutSystemUI>(new nite::Layout::Inline());    
  }
}

/*** [ PARSE METHOD ] 
    COLOR
***/
static nite::Color _parseColor(const String &name, const Jzon::Node &node, const nite::Color &contingency, Shared<nite::BaseUIComponent> &component){
  nite::Color result(contingency);
  if(!node.has(name)){
    return contingency;
  }
  auto obj = node.get(name);
  if(obj.isObject()){
    if(obj.has("r")){
      result.r = _parseGenericFloat(obj, "r", component);
    }
    if(obj.has("g")){
      result.g = _parseGenericFloat(obj, "g", component);
    }
    if(obj.has("b")){
      result.b = _parseGenericFloat(obj, "b", component);
    }
    if(obj.has("a")){
      result.a = _parseGenericFloat(obj, "a", component);
    }
  }else
  if(obj.isString()){
    result = nite::Color(obj.toString());
  }else{
    nite::print("Failed to parse JSON UI: "+name+" is not a color");
  }
  return result;
}

/*** [ PARSE METHOD ] 
    Any String
***/
static String _parseString(const String &name, Jzon::Node &node, const String &contingency, Shared<nite::BaseUIComponent> &component){
  String str = node.get(name).toString();
  return str == "" ? contingency : str;
}

/*** [ PARSE METHOD ] 
    Any Bool
***/
static bool _parseBool(const String &name, Jzon::Node &node, bool contingency, Shared<nite::BaseUIComponent> &component){
  return node.has(name) ? node.get(name).toBool() : contingency;
}

/*** [ PARSE METHOD ] 
    Any Int
***/
static int _parseInt(const String &name, Jzon::Node &node, int contingency, Shared<nite::BaseUIComponent> &component){
  return node.has(name) ? _parseGenericInt(node, name, component) : contingency;
}

/*** [ PARSE METHOD ] 
    Any Float
***/
static float _parseFloat(const String &name, Jzon::Node &node, float contingency, Shared<nite::BaseUIComponent> &component){
  return node.has(name) ? _parseGenericFloat(node, name, component) : contingency;
}

/*** [ PARSE METHOD ] 
    Texture/Image
***/
static nite::Texture _parseImage(const String &name, Jzon::Node &node, Shared<nite::BaseUIComponent> &component){
  if(!node.has(name) || (node.has(name) && node.get(name).isObject())){
    return nite::Texture();
  }

}

/*** [ PARSE METHOD ] 
    Margin/Padding
***/
static nite::Vec2 _parseDimensions(const String &name, Jzon::Node &node, const nite::Vec2 &contingency, Shared<nite::BaseUIComponent> &component){
  if(node.has(name) && node.get(name).isObject()){
    auto obj = node.get(name);
    return nite::Vec2( _parseFloat("hor", obj, contingency.x, component), _parseFloat("vert", obj, contingency.y, component));
  }else{
    return contingency;
  }
}

static Shared<nite::BaseUIComponent> _buildComponent(Jzon::Node &node, JsonSource &source){
  Shared<nite::BaseUIComponent> base = Shared<nite::BaseUIComponent>(new nite::PanelUI);
  String type = node.get("type").toString();

  if(type == "checkbox"){
    base = Shared<nite::BaseUIComponent>(new nite::CheckboxUI());
    auto *ref = static_cast<nite::CheckboxUI*>(base.get());
    auto text = node.get("text").toString();
    auto onChangeMethod = source.getListener(node.get("onChange").toString());
    auto onHoverMethod = source.getListener(node.get("onHover").toString());        
    auto fontColor = _parseColor("fontColor", node, ref->getFontColor(), base);
    auto selectorColor = _parseColor("selectorColor", node, ref->getSelectorColor(), base);
    auto boxColor = _parseColor("boxColor", node, ref->getBoxColor(), base);        
    auto size = _parseSize(node, nite::Vec2(-1.0f), base);  
    auto fontSize = nite::toInt(node.get("fontSize").toString());  
    auto flex = node.get("flex").toFloat();
    auto onUnhoverMethod = source.getListener(node.get("onUnhover").toString());
    auto margin = _parseDimensions("margin", node, nite::Vec2(0.0f), base);
    auto padding = _parseDimensions("padding", node, nite::Vec2(0.0f), base);
    ref->setMargin(margin);
    ref->setPadding(padding);    
    ref->setOnUnhover(onUnhoverMethod);     
    ref->setFlex(flex);    
    ref->setOnHover(onHoverMethod);
    ref->setSize(size);
    ref->setFontColor(fontColor);
    ref->setSelectorColor(selectorColor);
    ref->setBoxColor(boxColor);
    ref->setOnChange(onChangeMethod);
    ref->setFontSize(fontSize);
    ref->setText(text);
  }else
  if(type == "text"){
    base = Shared<nite::BaseUIComponent>(new nite::TextUI());
    auto *ref = static_cast<nite::TextUI*>(base.get());
    auto text = node.get("text").toString();
    auto textColor = _parseColor("fontColor", node, nite::Color(1.0f, 1.0f, 1.0f, 1.0f), base);
    auto fontSize = nite::toInt(node.get("fontSize").toString());
    auto size = _parseSize(node, nite::Vec2(-1.0f), base);     
    auto onClickMethod = source.getListener(node.get("onClick").toString());
    auto onHoverMethod = source.getListener(node.get("onHover").toString());
    auto flex = node.get("flex").toFloat();
    auto onUnhoverMethod = source.getListener(node.get("onUnhover").toString());
    auto margin = _parseDimensions("margin", node, nite::Vec2(0.0f), base);
    auto padding = _parseDimensions("padding", node, nite::Vec2(0.0f), base);
    auto shadowOffset = _parseDimensions("shadowOffset", node, ref->getShadowOffset(), base); 
    auto shadowColor = _parseColor("shadowColor", node, ref->getShadowColor(), base);     
    ref->setShadowColor(shadowColor);
    ref->setShadowOffset(shadowOffset);    
    ref->setMargin(margin);
    ref->setPadding(padding);        
    ref->setOnUnhover(onUnhoverMethod);    
    ref->setFlex(flex);         
    ref->setOnClick(onClickMethod);
    ref->setOnHover(onHoverMethod);
    ref->setSize(size);
    ref->setFontSize(fontSize);
    ref->setFontColor(textColor);    
    ref->setText(text);
  }else
  if(type == "button"){
    base = Shared<nite::BaseUIComponent>(new nite::ButtonUI());
    auto *ref = static_cast<nite::ButtonUI*>(base.get());
    auto text = node.get("text").toString();
    auto onClickMethod = source.getListener(node.get("onClick").toString());
    auto onHoverMethod = source.getListener(node.get("onHover").toString());
    auto size = _parseSize(node, nite::Vec2(-1.0f), base); 
    auto fontSize = _parseInt("fontSize", node, 14, base);
    auto baseColor = _parseColor("baseColor", node, ref->getBaseColor(), base); 
    auto fontColor = _parseColor("fontColor", node, ref->getFontColor(), base); 
    auto secondColor = _parseColor("secondColor", node, ref->getSecondColor(), base); 
    auto flex = node.get("flex").toFloat();
    auto onUnhoverMethod = source.getListener(node.get("onUnhover").toString());
    auto margin = _parseDimensions("margin", node, nite::Vec2(0.0f), base);
    auto padding = _parseDimensions("padding", node, nite::Vec2(8.0f), base);
    auto backgroundImage = _parseString("backgroundImage", node, "", base);
    ref->setMargin(margin);
    ref->setPadding(padding);        
    ref->setOnUnhover(onUnhoverMethod);        
    ref->setFlex(flex);    
    ref->setOnHover(onHoverMethod);
    ref->setBaseColor(baseColor);
    ref->setFontColor(fontColor);
    ref->setSecondColor(secondColor);
    ref->setFontSize(fontSize);       
    ref->setOnClick(onClickMethod);
    ref->setSize(size);
    ref->setText(text);
    if(backgroundImage != "" && nite::fileExists(backgroundImage)){
      ref->setBackgroundImage(nite::Texture(backgroundImage));
    }        
  }else
  if(type == "panel"){
    base = Shared<nite::BaseUIComponent>(new nite::PanelUI());
    auto *ref = static_cast<nite::PanelUI*>(base.get());
    auto layout = _parseLayout(node, base);
    auto onClickMethod = source.getListener(node.get("onClick").toString());
    auto onHoverMethod = source.getListener(node.get("onHover").toString());    
    auto backgroundColor = _parseColor("backgroundColor", node, nite::Color(1.0f, 1.0f, 1.0f, 0.0f), base);     
    auto size = _parseSize(node, nite::Vec2(16.0f), base); 
    auto flex = node.get("flex").toFloat();
    auto onUnhoverMethod = source.getListener(node.get("onUnhover").toString());
    auto margin = _parseDimensions("margin", node, nite::Vec2(0.0f), base);
    auto padding = _parseDimensions("padding", node, nite::Vec2(0.0f), base);
    ref->setMargin(margin);
    ref->setPadding(padding);        
    ref->setOnUnhover(onUnhoverMethod);        
    if(node.has("flex") && flex > 0.0f){
      ref->setFlex(flex);
    }else{
      ref->setFlex(0.0f);   
      ref->fillUpType = false;   
      ref->setSize(size);
    }
    ref->setBackgroundColor(backgroundColor);
    ref->setOnClick(onClickMethod);
    ref->setOnHover(onHoverMethod);
    ref->setLayout(layout);
  }else{
    nite::print("Parsing component without or undefined type from a UI JSON");    
  }
  // parse children
  auto _c = node.get("children");
  for(int i = 0; i < _c.getCount(); ++i){
    auto child = _c.get(i);
    base->add(_buildComponent(child, source));
  }  
  return base;
}


static void _build(Shared<nite::BaseUIComponent> &ui, const String &path, JsonSource &source){
  ui->clear(); // clear children  
  Jzon::Parser parser;
  
  // head is always parsed manually
  Jzon::Node node = parser.parseFile(path);  
  if(!node.isValid()){
    nite::print("Failed to build UI from '"+path+"': Invalid Json");
    return;
  }
  auto asWindow = static_cast<nite::WindowUI*>(ui.get());  
  auto size = _parseSize(node, nite::Vec2(300, 450), ui);
  auto layout = _parseLayout(node, ui);
  auto borderColor = _parseColor("borderColor", node, asWindow->getBorderColor(), ui);
  auto backgroundColor = _parseColor("backgroundColor", node, asWindow->getBackgroundColor(), ui);
  auto title = _parseString("title", node, asWindow->getTitle(), ui);
  auto noTitle = _parseBool("notitle", node, false, ui);
  auto unmovable = _parseBool("unmovable", node, false, ui);
  auto resizeable = _parseBool("resizeable", node, true, ui);
  auto borderthickness = _parseFloat("borderThickness", node, 8.0f, ui);
  auto borderPattern = _parseString("borderPattern", node, "", ui);
  auto backgroundImage = _parseString("backgroundImage", node, "", ui);
  if(backgroundImage != "" && nite::fileExists(backgroundImage)){
    asWindow->setBackgroundImage(nite::Texture(backgroundImage));
  }
  if(borderPattern != "" && nite::fileExists(borderPattern)){
    asWindow->setCornerPattern(nite::Texture(borderPattern, nite::Color(0.0f, 0.0f, 0.0f, 1.0f)));
  }
  asWindow->resizeable = resizeable;
  asWindow->unmovable = unmovable;
  asWindow->setBorderThickness(borderthickness);
  asWindow->setShowTitle(!noTitle);
  asWindow->setSize(size);
  asWindow->setLayout(layout);
  asWindow->setBorderColor(borderColor);
  asWindow->setBackgroundColor(backgroundColor);
  auto position = _parsePosition(node, nite::Vec2(0.0f, 0.0f), ui);    
  asWindow->setPosition(position);  
  asWindow->setTitle(title);

  auto _c = node.get("children");
  for(int i = 0; i < _c.getCount(); ++i){
    auto child = _c.get(i);
    asWindow->add(_buildComponent(child, source));
  }
}


// static UInt64 _l;
void nite::UIMaster::update(){
  // if(nite::getTicks()-_l > 1000){
  //   nite::print(sources.size());
  //   _l = nite::getTicks();
  // }
  for(int i = 0; i < sources.size(); ++i){
    auto &so = sources[i];
    if(nite::getTicks()-so.lastTick > 1000){
      so.lastTick = nite::getTicks();
      auto hash = nite::hashFile(so.path);
      if(hash != so.hash){
        nite::print("Changes on UI '"+so.path+"': reloading...");
        so.hash = hash;
        _build(so.ui, so.path, so);
      }
    }
  }
  for(int i = 0; i < components.size(); ++i){
    components[i]->updateRelativePosition(nite::Vec2(0.0f));
    components[i]->updateListeners();
    components[i]->beforeUpdate();
    components[i]->update();
    components[i]->afterUpdate();
  }

  auto removeFromSources = [&](nite::BaseUIComponent *comp){
    for(int i = 0; i < sources.size(); ++i){
      if(sources[i].ui.get() == comp){
        sources.erase(sources.begin() + i);
        return;
      }
    }
    return;
  };

  for(int i = 0; i < removeQueue.size(); ++i){
    for(int z = 0; z < components.size(); ++z){
      if(components[z].get() == removeQueue[i]){
        removeFromSources(removeQueue[i]);
        components[z]->clear();
        components.erase(components.begin() + z);
      }
    }
  }
  removeQueue.clear();
}

std::shared_ptr<nite::BaseUIComponent> nite::UIMaster::build(const String &path){
  return build(path, Dict<String, nite::ListenerLambda>());
}

std::shared_ptr<nite::BaseUIComponent> nite::UIMaster::build(const String &path, const Dict<String, nite::ListenerLambda> &listeners){
  JsonSource source;
  auto base = Shared<nite::BaseUIComponent>(new nite::WindowUI);
  add(base);
  source.listeners = listeners;  
  _build(base, path, source);
  source.path = path;
  source.hash = nite::hashFile(path);
  source.ui = base;
  sources.push_back(source);
  return base;
}