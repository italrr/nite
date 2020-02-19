#include "UI.hpp"
#include "../Tools/Tools.hpp"
#include "../Console.hpp"
#include "../../Game.hpp"

static Vector<std::shared_ptr<nite::BaseUIComponent>> components;
static Vector<nite::BaseUIComponent*> removeQueue;

/////////////
// COMMAND: ui_build
////////////
static nite::Console::Result cfBuildUI(Vector<String> params){
    static auto game = Game::getGameCoreInstance();
    if(params.size() < 1){
        return nite::Console::Result("Not enough parameters(1)", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
    }
    auto &path = params[0];
    if(!nite::fileExists(path)){
        return nite::Console::Result("'"+path+"' files does not exist.", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
    }
    auto win = static_cast<nite::WindowUI*>(nite::UI::build(path).get());
    return nite::Console::Result("Built '"+win->getTitle()+"' from '"+path+"' Id: "+nite::toStr(win->id), nite::Color(1.0f, 1.0f, 1.0f, 1.0f));

}
static auto cfBuildUIIns = nite::Console::CreateFunction("ui_build", &cfBuildUI); 


/////////////
// COMMAND: ui_show
////////////
static nite::Console::Result cfUInstances(Vector<String> params){
    static auto game = Game::getGameCoreInstance();
    String output;
    auto &comps = components;
    for(int i = 0; i < comps.size(); ++i){
        // it assumes all of the hosts are windows
        auto win = static_cast<nite::WindowUI*>(comps[i].get());
        output += "'"+win->getTitle()+"': "+nite::toStr(win->id);
        output += i < comps.size()-1 ? ", " : ".";
    }
    if(comps.size() == 0){
        output = "None.";
    }
    return nite::Console::Result("Active windows: "+output, nite::Color(1.0f, 1.0f, 1.0f, 1.0f));
}
static auto cfUInstancesIns = nite::Console::CreateFunction("ui_show", &cfUInstances); 

/////////////
// COMMAND: ui_kill
////////////
static nite::Console::Result cfUIKill(Vector<String> params){
	static auto game = Game::getGameCoreInstance();

	if(params.size() < 1){
		return nite::Console::Result("Not enough parameters(1)", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
	}
	auto &_id = params[0];
	if(!nite::isNumber(_id)){
		return nite::Console::Result("'"+_id+"' is not a valid parameter", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
	}	
	auto id = nite::toInt(_id);
	auto &comps = components;
	for(int i = 0; i < comps.size(); ++i){
		if(comps[i]->id == id){
			// it assumes all of the hosts are windows
			auto win = static_cast<nite::WindowUI*>(comps[i].get());			
			nite::print("Killed '"+win->getTitle()+"' Id was: "+nite::toStr(win->id));
			win->close();
			break;
		}
	}

}
static auto cfUiKillIns = nite::Console::CreateFunction("ui_kill", &cfUIKill); 
static auto cfUICloseIns = nite::Console::CreateFunction("ui_close", &cfUIKill); 

/////////////
// COMMAND: ui_rerender
////////////
// static nite::Console::Result cfUIRerender(Vector<String> params){
// 	auto &comps = components;
// 	for(int i = 0; i < comps.size(); ++i){
//         comps[i]->recalculate();
// 	}
// }
// static auto cfUIRerenderIns = nite::Console::CreateFunction("ui_rerender", &cfUIRerender); 

void nite::UI::add(std::shared_ptr<BaseUIComponent> comp){
    if(comp == NULL){
        return; 
    }
    for(int i = 0; i < components.size(); ++i){
        if(components[i].get() == comp.get()){
            nite::print("attempt to add a nite::BaseUIComponent twice in UI Master.");
            return;
        }
    }
    components.push_back(comp);
    // comp->currentMaster = this;
    comp->onCreate();
    comp->preinit();
}

void nite::UI::render(){
    for(int i = 0; i < components.size(); ++i){
        components[i]->beforeRender();
        components[i]->render();
        components[i]->afterRender();
    }
}

void nite::UI::remove(nite::BaseUIComponent *comp){
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

// example: "!! #gwidth + 0" expression always expects an expression, not single dynamic vars
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
                nite::print("failed to parse expression from UI JSON. token: '"+toktyp+"'");
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
            nite::print("failed to parse expression from UI JSON: unknown operator '"+op+"': syntax Error");
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
            nite::print("failed to parse expression from UI JSON: '"+input+"': syntax Error");
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

// complex parses
static nite::Vec2 _parseSize(const Jzon::Node &_node, Jzon::Node *style, const nite::Vec2 &contingency, Shared<nite::BaseUIComponent> &component){
    static const String name = "size";
    auto &node = _node.has(name) || style == NULL ? _node : *style;
    auto pnode = node.get(name);    
    float width = _parseGenericFloat(node, "width", component);
    float height = _parseGenericFloat(node, "height", component);
    return nite::Vec2(width == 0.0f ? contingency.x : width, height == 0.0f ? contingency.y : height);
}

static nite::Vec2 _parsePosition(const Jzon::Node &_node, Jzon::Node *style, const nite::Vec2 &contingency, Shared<nite::BaseUIComponent> &component){
    static const String name = "position";
    auto &node = _node.has(name) || style == NULL ? _node : *style;
    auto pnode = node.get(name);
    float width = _parseGenericFloat(pnode, "x", component);
    float height = _parseGenericFloat(pnode, "y", component);
    return nite::Vec2(width == 0.0f ? contingency.x : width, height == 0.0f ? contingency.y : height);
}

static Shared<nite::LayoutSystemUI> _parseLayout(const Jzon::Node &_node, Jzon::Node *style, Shared<nite::BaseUIComponent> &component){
    static const String name = "layout";
    auto &node = _node.has(name) || style == NULL ? _node : *style;
    String layout = node.get(name).toString();
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

static nite::Color _parseColor(const String &name, const Jzon::Node &_node, Jzon::Node *style, const nite::Color &contingency, Shared<nite::BaseUIComponent> &component){
    auto &node = _node.has(name) || style == NULL ? _node : *style;
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

static String _parseString(const String &name, Jzon::Node &_node, Jzon::Node *style, const String &contingency, Shared<nite::BaseUIComponent> &component){
    auto &node = _node.has(name) || style == NULL ? _node : *style;
    String str = node.get(name).toString();
    return str == "" ? contingency : str;
}

static bool _parseBool(const String &name, Jzon::Node &_node, Jzon::Node *style, bool contingency, Shared<nite::BaseUIComponent> &component){
    auto &node = _node.has(name) || style == NULL ? _node : *style;
    return node.has(name) ? node.get(name).toBool() : contingency;
}

static int _parseInt(const String &name, Jzon::Node &_node, Jzon::Node *style, int contingency, Shared<nite::BaseUIComponent> &component){
    auto &node = _node.has(name) || style == NULL ? _node : *style;
    return node.has(name) ? _parseGenericInt(node, name, component) : contingency;
}

static float _parseFloat(const String &name, Jzon::Node &_node, Jzon::Node *style, float contingency, Shared<nite::BaseUIComponent> &component){
    auto &node = _node.has(name) || style == NULL ? _node : *style;
    return node.has(name) ? _parseGenericFloat(node, name, component) : contingency;
}

static nite::Texture _parseImage(const String &name, Jzon::Node &_node, Jzon::Node *style, Shared<nite::BaseUIComponent> &component){
    auto &node = _node.has(name) || style == NULL ? _node : *style;
    if(!node.has(name) || (node.has(name) && node.get(name).isObject())){
        return nite::Texture();
    }
}

static nite::Vec2 _parseDimensions(const String &name, Jzon::Node &_node, Jzon::Node *style, const nite::Vec2 &contingency, Shared<nite::BaseUIComponent> &component){
    auto &node = _node.has(name) || style == NULL ? _node : *style;
    if(node.has(name) && node.get(name).isObject()){
        auto obj = node.get(name);
        return nite::Vec2( _parseFloat("hor", obj, style, contingency.x, component), _parseFloat("vert", obj, style, contingency.y, component));
    }else{
        return contingency;
    }
}

static Shared<nite::BaseUIComponent> _buildComponent(Jzon::Node &node, JsonSource &source, Dict<String, Shared<nite::BaseUIComponent>> &idsLookUp, Dict<String, Jzon::Node> &styles){
    Shared<nite::BaseUIComponent> base = Shared<nite::BaseUIComponent>(new nite::PanelUI);
    String type = node.get("type").toString();

    Jzon::Node *style = NULL;
    if(node.has("style") && node.get("style").isString()){
        auto it = styles.find(node.get("style").toString());
        if(it != styles.end()){
            style = &styles[it->first];
        }
    }

    if(type == "checkbox"){
        base = Shared<nite::BaseUIComponent>(new nite::CheckboxUI());
        auto *ref = static_cast<nite::CheckboxUI*>(base.get());
        auto text = node.get("text").toString();
        auto onChangeMethod = source.getListener(node.get("onChange").toString());
        auto onHoverMethod = source.getListener(node.get("onHover").toString());        
        auto fontColor = _parseColor("fontColor", node, style, ref->getFontColor(), base);
        auto selectorColor = _parseColor("selectorColor", node, style, ref->getSelectorColor(), base);
        auto boxColor = _parseColor("boxColor", node, style, ref->getBoxColor(), base);        
        auto size = _parseSize(node, style, nite::Vec2(-1.0f), base);  
        auto fontSize = _parseInt("fontSize", node, style, ref->getFontSize(), base);
        auto flex = _parseFloat("flex", node, style, 0.0f, base);
        auto onUnhoverMethod = source.getListener(node.get("onUnhover").toString());
        auto margin = _parseDimensions("margin", node, style, nite::Vec2(0.0f), base);
        auto padding = _parseDimensions("padding", node, style, nite::Vec2(0.0f), base);
        auto id = _parseString("id", node, style, base->literalId, base);
        ref->setId(id);
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
        auto textColor = _parseColor("fontColor", node, style, nite::Color(1.0f, 1.0f, 1.0f, 1.0f), base);
        auto fontSize = _parseInt("fontSize", node, style, ref->getFontSize(), base);
        auto size = _parseSize(node, style, nite::Vec2(-1.0f), base);     
        auto onClickMethod = source.getListener(node.get("onClick").toString());
        auto onHoverMethod = source.getListener(node.get("onHover").toString());
        auto flex = _parseFloat("flex", node, style, 0.0f, base);
        auto onUnhoverMethod = source.getListener(node.get("onUnhover").toString());
        auto margin = _parseDimensions("margin", node, style, nite::Vec2(0.0f), base);
        auto padding = _parseDimensions("padding", node, style, nite::Vec2(0.0f), base);
        auto shadowOffset = _parseDimensions("shadowOffset", node, style, ref->getShadowOffset(), base); 
        auto shadowColor = _parseColor("shadowColor", node, style, ref->getShadowColor(), base);  
        auto id = _parseString("id", node, style, base->literalId, base);
        ref->setId(id);       
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
        auto fontSize = _parseInt("fontSize", node, style, ref->getFontSize(), base);
        auto onClickMethod = source.getListener(node.get("onClick").toString());
        auto onHoverMethod = source.getListener(node.get("onHover").toString());
        auto size = _parseSize(node, style, nite::Vec2(-1.0f), base); 
        auto baseColor = _parseColor("baseColor", node, style, ref->getBaseColor(), base); 
        auto fontColor = _parseColor("fontColor", node, style, ref->getFontColor(), base); 
        auto secondColor = _parseColor("secondColor", node, style, ref->getSecondColor(), base); 
        auto flex = _parseFloat("flex", node, style, 0.0f, base);
        auto onUnhoverMethod = source.getListener(node.get("onUnhover").toString());
        auto margin = _parseDimensions("margin", node, style, nite::Vec2(0.0f), base);
        auto padding = _parseDimensions("padding", node, style, nite::Vec2(8.0f), base);
        auto backgroundImage = _parseString("backgroundImage", node, NULL, "", base);
        auto id = _parseString("id", node, style, base->literalId, base);
        ref->setId(id);    
        ref->setMargin(margin);
        ref->setPadding(padding);        
        ref->setOnUnhover(onUnhoverMethod);        
        ref->setFlex(flex);    
        ref->setFontSize(fontSize);
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
        auto layout = _parseLayout(node, style, base);
        auto onClickMethod = source.getListener(node.get("onClick").toString());
        auto onHoverMethod = source.getListener(node.get("onHover").toString());    
        auto backgroundColor = _parseColor("backgroundColor", node, style, nite::Color(1.0f, 1.0f, 1.0f, 0.0f), base);     
        auto size = _parseSize(node, style, nite::Vec2(16.0f), base); 
        auto flex = _parseFloat("flex", node, style, ref->flex, base);
        auto onUnhoverMethod = source.getListener(node.get("onUnhover").toString());
        auto margin = _parseDimensions("margin", node, style, nite::Vec2(0.0f), base);
        auto padding = _parseDimensions("padding", node, style, nite::Vec2(0.0f), base);
        auto id = _parseString("id", node, style, base->literalId, base);
        ref->setId(id);    
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
        nite::print("parsing component without/undefined type from a UI JSON");    
    }
    // parse children
    auto _c = node.get("children");
    for(int i = 0; i < _c.getCount(); ++i){
        auto child = _c.get(i);
        auto comp = _buildComponent(child, source, idsLookUp, styles);
        idsLookUp[comp->literalId] = comp; 
        base->add(comp);
    }  
    return base;
}


static void _build(Shared<nite::BaseUIComponent> &ui, const String &path, JsonSource &source){
    ui->clear(); // clear children  
    Jzon::Parser parser;
    nite::print("building ui '"+path+"'...");
    // head is always parsed manually
    Jzon::Node node = parser.parseFile(path);  
    if(!node.isValid()){
        nite::print("failed to build UI from '"+path+"': invalid Json");
        return;
    }
    auto asWindow = static_cast<nite::WindowUI*>(ui.get());  
    auto size = _parseSize(node, NULL, nite::Vec2(300, 450), ui);
    auto layout = _parseLayout(node, NULL, ui);
    auto borderColor = _parseColor("borderColor", node, NULL,asWindow->getBorderColor(), ui);
    auto backgroundColor = _parseColor("backgroundColor", node, NULL, asWindow->getBackgroundColor(), ui);
    auto titleColor = _parseColor("titleColor", node, NULL,asWindow->getTitleColor(), ui);
    auto title = _parseString("title", node, NULL, asWindow->getTitle(), ui);
    auto noTitle = _parseBool("notitle", node, NULL, false, ui);
    auto unmovable = _parseBool("unmovable", node, NULL, false, ui);
    auto resizeable = _parseBool("resizeable", node, NULL, true, ui);
    auto borderthickness = _parseFloat("borderThickness", node, NULL, 8.0f, ui);
    auto borderPattern = _parseString("borderPattern", node, NULL, "", ui);
    auto backgroundImage = _parseString("backgroundImage", node, NULL, "", ui);
    auto position = _parsePosition(node, NULL, nite::Vec2(0.0f, 0.0f), ui);    
    auto id = _parseString("id", node, NULL, asWindow->literalId, ui);
    asWindow->setId(id);  
    if(backgroundImage != "" && nite::fileExists(backgroundImage)){
        asWindow->setBackgroundImage(nite::Texture(backgroundImage));
    }
    if(borderPattern != "" && nite::fileExists(borderPattern)){
        asWindow->setCornerPattern(nite::Texture(borderPattern, nite::Color(0.0f, 0.0f, 0.0f, 1.0f)));
    }
    if(node.has("styles") && node.get("styles").isObject()){
        auto styles = node.get("styles");
        for(auto &style : styles){
            asWindow->styles[style.first] = style.second;
        }
    }
    asWindow->resizeable = resizeable;
    asWindow->unmovable = unmovable;
    asWindow->setBorderThickness(borderthickness);
    asWindow->setShowTitle(!noTitle);
    asWindow->setSize(size);
    asWindow->setTitleColor(titleColor);
    asWindow->setLayout(layout);
    asWindow->setBorderColor(borderColor);
    asWindow->setBackgroundColor(backgroundColor);  
    asWindow->setPosition(position);  
    asWindow->setTitle(title);

    auto _c = node.get("children");
    for(int i = 0; i < _c.getCount(); ++i){
        auto child = _c.get(i);
        asWindow->add(_buildComponent(child, source, asWindow->idsLookUp, asWindow->styles));
    }
    nite::print("built ui '"+path+"' | "+nite::toStr(source.listeners.size())+" listener(s) | "+nite::toStr(asWindow->styles.size())+" style(s)");
}


// static UInt64 _l;
void nite::UI::update(){
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
                nite::print("changes on UI '"+so.path+"': reloading...");
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
                components[z]->id = -1;
                removeFromSources(removeQueue[i]);
                components[z]->clear();
                components.erase(components.begin() + z);
            }
        }
    }
    removeQueue.clear();
}

std::shared_ptr<nite::BaseUIComponent> nite::UI::build(const String &path){
    return build(path, Dict<String, nite::ListenerLambda>());
}

static int getUniqueId(){
	static int seed = 1 + nite::randomInt(15, 35);
	return ++seed;
}

std::shared_ptr<nite::BaseUIComponent> nite::UI::build(const String &path, const Dict<String, nite::ListenerLambda> &listeners){
    JsonSource source;
    auto base = Shared<nite::BaseUIComponent>(new nite::WindowUI);
	base->id = getUniqueId();
    add(base);
    source.listeners = listeners;  
    _build(base, path, source);
    source.path = path;
    source.hash = nite::hashFile(path);
    source.ui = base;
    sources.push_back(source);
    return base;
}