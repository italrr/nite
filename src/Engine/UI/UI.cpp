#include <string.h>
#include "UI.hpp"
#include "../Tools/Tools.hpp"
#include "../Console.hpp"
#include "../Input.hpp"
// #include "../../Game.hpp"

static Vector<std::shared_ptr<nite::BaseUIComponent>> components;
static Vector<nite::BaseUIComponent*> removeQueue;

static int getUniqueId(){
	static int seed = 1 + nite::randomInt(15, 35);
	return ++seed;
}

void nite::UI::ValueChangeListener::update(){
    if(target == NULL || type == ValueChangeListenerType::None){
        return;
    }
    changed = false;
    switch(type){
        case ValueChangeListenerType::Intenger: {
            Int32 &a = *static_cast<Int32*>(target);
            Int32 &b = *static_cast<Int32*>((void*)lastVal);
            if(a != b){
                onChange(target);
                memset(lastVal, '0', NITE_VALUE_CHANGE_LISTENER_LASTVAL_SIZE);
                memcpy(lastVal, target, sizeof(Int32));                   
            }
        } break;
        case ValueChangeListenerType::UInteger: {
            UInt32 &a = *static_cast<UInt32*>(target);
            UInt32 &b = *static_cast<UInt32*>((void*)lastVal);
            if(a != b){
                onChange(target);
                memset(lastVal, '0', NITE_VALUE_CHANGE_LISTENER_LASTVAL_SIZE);
                memcpy(lastVal, target, sizeof(UInt32));                 
            }            
        } break;            
        case ValueChangeListenerType::Float: {
            float &a = *static_cast<float*>(target);
            float &b = *static_cast<float*>((void*)lastVal);
            if(a != b){
                onChange(target);
                memset(lastVal, '0', NITE_VALUE_CHANGE_LISTENER_LASTVAL_SIZE);
                memcpy(lastVal, target, sizeof(float));                
            }            
        } break;        
        case ValueChangeListenerType::Literal: {
            String &a = *static_cast<String*>(target);
            String b(static_cast<char*>((void*)lastVal));
            if(a != b){
                onChange(target);
                memset(lastVal, '0', NITE_VALUE_CHANGE_LISTENER_LASTVAL_SIZE);
                memcpy(lastVal, a.c_str(), a.size() + 1);
            }              
        } break;        
    }
}

nite::UI::ValueChangeListener::ValueChangeListener(){
    clear();
}

void nite::UI::ValueChangeListener::clear(){
    type = ValueChangeListenerType::None;
    target = NULL;
    onChange = [](void *nv){

    };
    changed = false;
    memset(lastVal, '0', NITE_VALUE_CHANGE_LISTENER_LASTVAL_SIZE);
}

void nite::UI::ValueChangeListener::listen(Int32 *v){
    type = ValueChangeListenerType::Intenger;
    target = v;
    memcpy(lastVal, v, sizeof(Int32));
}

void nite::UI::ValueChangeListener::listen(UInt32 *v){
    type = ValueChangeListenerType::UInteger;
    target = v;
    memcpy(lastVal, v, sizeof(UInt32));
}

void nite::UI::ValueChangeListener::listen(String *v){
    type = ValueChangeListenerType::Literal;
    target = v;
    memcpy(lastVal, v->c_str(), v->size() + 1); // include nulltermination
}

void nite::UI::ValueChangeListener::listen(float *v){
    type = ValueChangeListenerType::Float;
    target = v;
    memcpy(lastVal, v, sizeof(float));
}

/////////////
// COMMAND: ui_build
////////////
static nite::Console::Result cfBuildUI(Vector<String> params){
    // static auto game = Game::getGameCoreInstance();
    if(params.size() < 1){
        return nite::Console::Result("not enough parameters(1)", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
    }
    auto &path = params[0];
    if(!nite::fileExists(path)){
        return nite::Console::Result("'"+path+"' files does not exist.", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
    }
    auto win = static_cast<nite::WindowUI*>(nite::UI::build(path).get());
    return nite::Console::Result("built '"+win->getTitle()+"' from '"+path+"' Id: "+nite::toStr(win->id), nite::Color(1.0f, 1.0f, 1.0f, 1.0f));

}
static auto cfBuildUIIns = nite::Console::CreateFunction("ui_build", &cfBuildUI); 


/////////////
// COMMAND: ui_show
////////////
static nite::Console::Result cfUInstances(Vector<String> params){
    // static auto game = Game::getGameCoreInstance();
    String output;
    auto &comps = components;
    for(int i = 0; i < comps.size(); ++i){
        // it assumes all of the hosts are windows
        auto win = static_cast<nite::WindowUI*>(comps[i].get());
        output += "'"+win->getTitle()+"': "+nite::toStr(win->id);
        output += i < comps.size()-1 ? ", " : ".";
    }
    if(comps.size() == 0){
        output = "none";
    }
    return nite::Console::Result("active windows: "+output, nite::Color(1.0f, 1.0f, 1.0f, 1.0f));
}
static auto cfUInstancesIns = nite::Console::CreateFunction("ui_show", &cfUInstances); 

/////////////
// COMMAND: ui_kill
////////////
static nite::Console::Result cfUIKill(Vector<String> params){
	// static auto game = Game::getGameCoreInstance();

	if(params.size() < 1){
		return nite::Console::Result("not enough parameters(1)", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
	}
	auto &_id = params[0];
	if(!nite::isNumber(_id)){
		return nite::Console::Result("'"+_id+"' is not a valid parameter", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
	}	
	auto id = nite::toInt(_id);
	auto &comps = components;
	for(int i = 0; i < comps.size(); ++i){
		if(comps[i]->id == id){
			//it assumes all of the hosts are windows (they must be)
			auto win = static_cast<nite::WindowUI*>(comps[i].get());			
			win->close();
			return nite::Console::Result("killed '"+win->getTitle()+"' id was: "+nite::toStr(win->id), nite::Color(1.0f, 1.0f, 1.0f, 1.0f));
		}
	}
    return nite::Console::Result();
}
static auto cfUiKillIns = nite::Console::CreateFunction("ui_kill", &cfUIKill); 
static auto cfUICloseIns = nite::Console::CreateFunction("ui_close", &cfUIKill); 

/////////////
// COMMAND: ui_rerender
////////////
static nite::Console::Result cfUIRerender(Vector<String> params){
	auto &comps = components;
	for(int i = 0; i < comps.size(); ++i){
        comps[i]->rerender();
	}
    return nite::Console::Result("true", nite::Color(1.0f, 1.0f, 1.0f, 1.0f));
}
static auto cfUIRerenderIns = nite::Console::CreateFunction("ui_rerender", &cfUIRerender); 

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
        if(!components[i]->visible){
            continue;
        }        
        components[i]->beforeRender();
        components[i]->render(nite::Vec2(0.0f));
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
    Dict<String, Shared<nite::BaseUIComponent>> idsLookUp;
    Shared<nite::BaseUIComponent> ui;
    Dict<String, nite::ListenerLambda> listeners;
    nite::ListenerLambda getListener(const String &id){
        auto dummy = [](const Shared<nite::ListenerInfo> &info, const nite::BaseUIComponent *component){
            return;
        };
        return listeners.find(id) != listeners.end() ? listeners[id] : dummy;
    }
};

static Vector<JsonSource> sources;

// example: "!! #gwidth + 0" "expression" always expects an expression, not single dynamic vars
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
        if(tok == "hwidth"){
            return component->getTopHeadComponent()->computeSize().x;
        }
        if(tok == "hheight"){
            return component->getTopHeadComponent()->computeSize().y;
        }        
        return 0.0f;
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
    // TODO: apply PEDMAS here
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
struct _RelSizeInfo {
    bool isX;
    bool isY;
    nite::Vec2 size;
    _RelSizeInfo(){
        this->isX = false;
        this->isY = false;
    }
};

static inline float _parseRelSize(const String &v){
    return v.length() > 0 && v[v.length()-1] == '%' ? nite::toFloat(v.substr(0, v.length()-1)) / 100.0f : -1.0f;
}

// TODO: improve this method. it's ugly as sin
static _RelSizeInfo _parseSize(const Jzon::Node &_node, Jzon::Node *style, const nite::Vec2 &contingency, Shared<nite::BaseUIComponent> &component){
    static const String name = "size";
    auto node = _node;

    if(style != NULL){
        for(auto &it : *style){
            node.add(it.first, it.second);
        }
    }

    _RelSizeInfo rv;
    
    auto wn = node.get("width");
    auto hn = node.get("height");

    if(node.has("width") && wn.isNumber()){
        rv.size.x = _parseGenericFloat(node, "width", component);
    }else
    if(node.has("width") && wn.isString()){
        auto str = wn.toString();
        if(str[str.length()-1] == '%'){
            rv.size.x = _parseRelSize(wn.toString());
            rv.isX = true;
        }else{
            rv.size.x = _parseGenericFloat(node, "width", component);
        }
    }else{
        rv.size.x = contingency.x;
    }

    if(node.has("height") && hn.isNumber()){
        rv.size.y = _parseGenericFloat(node, "height", component);
    }else
    if(node.has("height") && hn.isString()){
        auto str = hn.toString();
        if(str[str.length()-1] == '%'){        
            rv.size.y = _parseRelSize(hn.toString());
            rv.isY = true;
        }else{
            rv.size.y = _parseGenericFloat(node, "height", component);
        }
    }else{
        rv.size.y = contingency.y;
    }    

    return rv;
}

static nite::Vec2 _parsePosition(const Jzon::Node &_node, Jzon::Node *style, const nite::Vec2 &contingency, Shared<nite::BaseUIComponent> &component){
    static const String name = "position";
    auto node = _node;
    if(style != NULL){
        for(auto &it : *style){
            node.add(it.first, it.second);
        }
    }
    auto pnode = node.get(name);
    float width = _parseGenericFloat(pnode, "x", component);
    float height = _parseGenericFloat(pnode, "y", component);
    return nite::Vec2(width == 0.0f ? contingency.x : width, height == 0.0f ? contingency.y : height);
}

static Shared<nite::LayoutSystemUI> _parseLayout(const Jzon::Node &_node, Jzon::Node *style, Shared<nite::BaseUIComponent> &component){
    static const String name = "layout";
    auto node = _node;
    if(style != NULL){
        for(auto &it : *style){
            node.add(it.first, it.second);
        }
    }
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
    return Shared<nite::LayoutSystemUI>(NULL);
}

static nite::Color _parseColor(const String &name, const Jzon::Node &_node, Jzon::Node *style, const nite::Color &contingency, Shared<nite::BaseUIComponent> &component){
    auto node = _node;
    if(style != NULL){
        for(auto &it : *style){
            node.add(it.first, it.second);
        }
    }
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
    auto node = _node;
    if(style != NULL){
        for(auto &it : *style){
            node.add(it.first, it.second);
        }
    }
    String str = node.get(name).toString();
    return str == "" ? contingency : str;
}

static bool _parseBool(const String &name, Jzon::Node &_node, Jzon::Node *style, bool contingency, Shared<nite::BaseUIComponent> &component){
    auto node = _node;
    if(style != NULL){
        for(auto &it : *style){
            node.add(it.first, it.second);
        }
    }
    return node.has(name) ? node.get(name).toBool() : contingency;
}

static int _parseInt(const String &name, Jzon::Node &_node, Jzon::Node *style, int contingency, Shared<nite::BaseUIComponent> &component){
    auto node = _node;
    if(style != NULL){
        for(auto &it : *style){
            node.add(it.first, it.second);
        }
    }
    return node.has(name) ? _parseGenericInt(node, name, component) : contingency;
}

static float _parseFloat(const String &name, Jzon::Node &_node, Jzon::Node *style, float contingency, Shared<nite::BaseUIComponent> &component){
    auto node = _node;
    if(style != NULL){
        for(auto &it : *style){
            node.add(it.first, it.second);
        }
    }
    return node.has(name) ? _parseGenericFloat(node, name, component) : contingency;
}

static nite::Texture _parseImage(const String &name, Jzon::Node &_node, Jzon::Node *style, Shared<nite::BaseUIComponent> &component){
    auto node = _node;
    if(style != NULL){
        for(auto &it : *style){
            node.add(it.first, it.second);
        }
    }
    if(!node.has(name) || (node.has(name) && node.get(name).isObject())){
        return nite::Texture();
    }
    return nite::Texture();
}

struct ShaderPair {
    String vert;
    String frag;
    bool success;
    ShaderPair(){
        success = false;
    }
};

static ShaderPair _parseShader(const String &name, Jzon::Node &_node, Jzon::Node *style, Shared<nite::BaseUIComponent> &component){
    auto node = _node;
    if(style != NULL){
        for(auto &it : *style){
            node.add(it.first, it.second);
        }
    }
    ShaderPair shader;
    if(node.has(name) && node.get(name).isObject()){
        String vert = node.get(name).get("vert").toString();
        String frag = node.get(name).get("frag").toString();
        if(nite::fileExists(vert) && nite::fileExists(frag)){
            shader.success = true;
            shader.vert = vert;
            shader.frag = frag;
        }
    }
    return shader;    
}


static nite::Vec2 _parseDimensions(const String &name, Jzon::Node &_node, Jzon::Node *style, const nite::Vec2 &contingency, Shared<nite::BaseUIComponent> &component){
    auto node = _node;
    if(style != NULL){
        for(auto &it : *style){
            node.add(it.first, it.second);
        }
    }
    if(node.has(name) && node.get(name).isObject()){
        auto obj = node.get(name);
        return nite::Vec2( _parseFloat("hor", obj, style, contingency.x, component), _parseFloat("vert", obj, style, contingency.y, component));
    }else{
        return contingency;
    }
}


static void _parseNav(const String &name, Jzon::Node &node, Shared<nite::BaseUIComponent> &component){
    if(node.has(name) && node.get(name).isObject()){
        auto nav = node.get(name);
        component->nav.index = nav.get("index").toInt(0);
        component->nav.enable = nav.get("enable").toBool(false);
        component->nav.split = _parseInt("split", nav, NULL, 0, component);
    }
}

static void _parseListenOn(const String &name, Jzon::Node &node, Dict<String, nite::ListenerLambda> &_listeners, Shared<nite::BaseUIComponent> &component){
    if(node.has(name) && node.get(name).isObject()){
        auto listeners = node.get(name);
        for(auto &list : listeners){
            auto it = _listeners.find(list.second.toString());
            if(it != _listeners.end()){
                component->keyListeners[list.first] = it->second;
            }
        }
    }
}

static void _parseOverflow(const String &name, Jzon::Node &node, Shared<nite::BaseUIComponent> &component){
    if(node.has(name) && node.get(name).isString()){
        auto overflow = node.get(name).toString();
        if(overflow == "scroll-x"){
            component->allowOverflow = true;
            component->scrollX = true;
        }else
        if(overflow == "scroll-y"){
            component->allowOverflow = true;
            component->scrollY = true;            
        }else   
        if(overflow == "scroll-xy" || overflow == "scroll-bidirectional"){
            component->allowOverflow = true;
            component->scrollX = true;               
            component->scrollY = true;   
        }             
    }
}


// build from jzon node

Shared<nite::BaseUIComponent> nite::UI::build(Jzon::Node &node, Dict<String, Jzon::Node> &styles, Dict<String, nite::ListenerLambda> &listeners, Dict<String, Shared<nite::BaseUIComponent>> &idsLookUp){
    Shared<nite::BaseUIComponent> base = Shared<nite::BaseUIComponent>(new nite::PanelUI);
    String type = node.get("type").toString();

    auto getListener = [&](const String &id){
        auto dummy = [](const Shared<nite::ListenerInfo> &info, const nite::BaseUIComponent *component){
            return;
        };
        return listeners.find(id) != listeners.end() ? listeners[id] : dummy;
    };

    Jzon::Node *style = NULL;
    if(node.has("style") && node.get("style").isString()){
        auto it = styles.find(node.get("style").toString());
        if(it != styles.end()){
            style = &styles[it->first];
        }
    }

    if(type == "window" && node.has("styles") && node.get("styles").isObject()){
        auto styls = node.get("styles");
        for(auto &style : styls){
            styles[style.first] = style.second;
        }
    }  

    if(node.has("import") && node.get("import").isString()){
        String path = node.get("import").toString();
        if(nite::fileExists(path)){
            Jzon::Parser parser;
            auto imported = parser.parseFile(path);         
            if(!imported.isValid()){
                nite::print("failed to UI import '"+path+"': "+parser.getError());
            }else{
                nite::print("UI import '"+path+"'");
            }
            for(auto item : node){
                if(item.first == "import"){
                    continue;
                }
                imported.add(item.first, item.second);
            }
            base = nite::UI::build(imported, styles, listeners, idsLookUp);
            return base;
        }else{
            base = Shared<nite::BaseUIComponent>(new nite::TextUI());
            auto *ref = static_cast<nite::TextUI*>(base.get());
            ref->setText("failed import");
            nite::print("failed to UI import '"+path+"': it doesn't exist");
        }
    }else
    if(type == "window"){
        base = Shared<nite::BaseUIComponent>(new nite::WindowUI());
        base->styles = styles; // for now only window is allowed to keep a copy this
        auto *ref = static_cast<nite::WindowUI*>(base.get());
        auto size = _parseSize(node, NULL, nite::Vec2(300, 450), base);
        auto layout = _parseLayout(node, NULL, base);
        auto borderColor = _parseColor("borderColor", node, NULL,ref->getBorderColor(), base);
        auto backgroundColor = _parseColor("backgroundColor", node, NULL, ref->getBackgroundColor(), base);
        auto titleColor = _parseColor("titleColor", node, NULL,ref->getTitleColor(), base);
        auto title = _parseString("title", node, NULL, ref->getTitle(), base);
        auto noTitle = _parseBool("notitle", node, NULL, false, base);
        auto unmovable = _parseBool("unmovable", node, NULL, false, base);
        auto resizeable = _parseBool("resizeable", node, NULL, true, base);
        auto modal = _parseBool("modal", node, NULL, false, base);
        auto borderthickness = _parseFloat("borderThickness", node, NULL, 8.0f, base);
        auto borderPattern = _parseString("borderPattern", node, NULL, "", base);
        auto backgroundImage = _parseString("backgroundImage", node, NULL, "", base);
        auto position = _parsePosition(node, NULL, nite::Vec2(0.0f, 0.0f), base);    
        auto center = _parseBool("center", node, NULL, false, base);
        auto fontSize = _parseInt("fontSize", node, style, ref->getFontSize(), base);
        auto font = _parseString("font", node, style, "", base);        
        // auto userShader = _parseShader("shader", node, NULL, base);
        _parseOverflow("overflow", node, base);
        _parseNav("navigate", node, base);
        _parseListenOn("listenOn", node, listeners, base);
        // if(userShader.success){
        //     base->apply(nite::Shader(userShader.frag, userShader.vert));
        // }
        if(font != "" && nite::fileExists(font)){
            ref->setFont(nite::Font(font,  fontSize));
        }          
        auto id = _parseString("id", node, NULL, ref->literalId, base);
        ref->setId(id);  
        if(backgroundImage != "" && nite::fileExists(backgroundImage)){
            ref->setBackgroundImage(nite::Texture(backgroundImage));
        }
        if(borderPattern != "" && nite::fileExists(borderPattern)){
            ref->setCornerPattern(nite::Texture(borderPattern, nite::Color(0.0f, 0.0f, 0.0f, 1.0f)));
        }
        auto clickAnalogue = _parseString("clickAnalogueKey", node, NULL, "W", base);
        auto snapInterpRate = _parseFloat("snapInterpRate", node, style, ref->snapInterpRate, base);
        ref->onClickAnalogueKey = nite::translateKey(clickAnalogue);        
        ref->setSnapInterpRate(snapInterpRate);        
        ref->resizeable = resizeable;
        ref->setCenter(center);
        ref->unmovable = unmovable;
        ref->setModal(modal);
        ref->setBorderThickness(borderthickness);
        ref->setShowTitle(!noTitle);
        ref->setSize(size.size);
        ref->useRelSizeX = size.isX;
        ref->useRelSizeY = size.isY;
        ref->setFontSize(fontSize);        
        ref->setTitleColor(titleColor);
        ref->setLayout(layout);
        ref->setBorderColor(borderColor);
        ref->setBackgroundColor(backgroundColor);  
        ref->setPosition(position);  
        ref->setTitle(title);
    }else
    if(type == "checkbox"){
        base = Shared<nite::BaseUIComponent>(new nite::CheckboxUI());
        auto *ref = static_cast<nite::CheckboxUI*>(base.get());
        auto text = node.get("text").toString();
        auto onChangeMethod = getListener(node.get("onChange").toString());
        auto onHoverMethod = getListener(node.get("onHover").toString());        
        auto fontColor = _parseColor("fontColor", node, style, ref->getFontColor(), base);
        auto selectorColor = _parseColor("selectorColor", node, style, ref->getSelectorColor(), base);
        auto boxColor = _parseColor("boxColor", node, style, ref->getBoxColor(), base);        
        auto font = _parseString("font", node, style, "", base); 
        auto size = _parseSize(node, style, nite::Vec2(-1.0f), base);  
        auto fontSize = _parseInt("fontSize", node, style, ref->getFontSize(), base);
        auto flex = _parseFloat("flex", node, style, 0.0f, base);
        auto onUnhoverMethod = getListener(node.get("onUnhover").toString());
        auto margin = _parseDimensions("margin", node, style, nite::Vec2(0.0f), base);
        auto padding = _parseDimensions("padding", node, style, nite::Vec2(0.0f), base);
        auto id = _parseString("id", node, style, base->literalId, base);
        // auto userShader = _parseShader("shader", node, style, base);
        _parseNav("navigate", node, base);
        _parseListenOn("listenOn", node, listeners, base);
        // if(userShader.success){
        //     base->apply(nite::Shader(userShader.frag, userShader.vert));
        // }        
        if(font != "" && nite::fileExists(font)){
            ref->setFont(nite::Font(font,  fontSize));
        }
        auto snapInterpRate = _parseFloat("snapInterpRate", node, style, ref->snapInterpRate, base);
        ref->setSnapInterpRate(snapInterpRate);                  
        ref->setId(id);
        ref->setMargin(margin);
        ref->setPadding(padding);    
        ref->setOnUnhover(onUnhoverMethod);     
        ref->setFlex(flex);    
        ref->setOnHover(onHoverMethod);
        if(node.has("flex") && flex > 0.0f){
            ref->setFlex(flex);
        }else{
            ref->setFlex(0.0f);   
            ref->fillUpType = false;   
            ref->setSize(size.size);
            ref->useRelSizeX = size.isX;
            ref->useRelSizeY = size.isY;
        }  
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
        // auto size = _parseSize(node, style, nite::Vec2(-1.0f), base);     
        auto onClickMethod = getListener(node.get("onClick").toString());
        auto onHoverMethod = getListener(node.get("onHover").toString());
        auto flex = _parseFloat("flex", node, style, 0.0f, base);
        auto onUnhoverMethod = getListener(node.get("onUnhover").toString());
        auto margin = _parseDimensions("margin", node, style, nite::Vec2(0.0f), base);
        auto padding = _parseDimensions("padding", node, style, nite::Vec2(0.0f), base);
        auto shadowOffset = _parseDimensions("shadowOffset", node, style, ref->getShadowOffset(), base); 
        auto shadowColor = _parseColor("shadowColor", node, style, ref->getShadowColor(), base);  
        auto id = _parseString("id", node, style, base->literalId, base);
        auto font = _parseString("font", node, style, "", base); 
        // auto userShader = _parseShader("shader", node, style, base);
        _parseNav("navigate", node, base);
        _parseListenOn("listenOn", node, listeners, base);
        // if(userShader.success){
        //     base->apply(nite::Shader(userShader.frag, userShader.vert));
        // }         
        if(font != "" && nite::fileExists(font)){
            ref->setFont(nite::Font(font,  fontSize));
        }  
        auto snapInterpRate = _parseFloat("snapInterpRate", node, style, ref->snapInterpRate, base);
        ref->setSnapInterpRate(snapInterpRate);                     
        ref->setId(id);       
        ref->setShadowColor(shadowColor);
        ref->setShadowOffset(shadowOffset);    
        ref->setMargin(margin);
        ref->setPadding(padding);        
        ref->setOnUnhover(onUnhoverMethod);    
        ref->setFlex(flex);         
        ref->setOnClick(onClickMethod);
        ref->setOnHover(onHoverMethod);
        if(node.has("flex") && flex > 0.0f){
            ref->setFlex(flex);
        }else{
            ref->setFlex(0.0f);   
            ref->fillUpType = false;   
            // ref->setSize(size.size);
            // ref->relSize.set(size.size);
            // ref->useRelSizeX = size.isX;
            // ref->useRelSizeY = size.isY;
        }  
        ref->setFontSize(fontSize);
        ref->setFontColor(textColor);    
        ref->setText(text);
    }else
    if(type == "button"){
        base = Shared<nite::BaseUIComponent>(new nite::ButtonUI());
        auto *ref = static_cast<nite::ButtonUI*>(base.get());
        auto text = node.get("text").toString();
        auto fontSize = _parseInt("fontSize", node, style, ref->getFontSize(), base);
        auto onClickMethod = getListener(node.get("onClick").toString());
        auto onHoverMethod = getListener(node.get("onHover").toString());
        auto size = _parseSize(node, style, nite::Vec2(-1.0f), base); 
        auto baseColor = _parseColor("baseColor", node, style, ref->getBaseColor(), base); 
        auto fontColor = _parseColor("fontColor", node, style, ref->getFontColor(), base); 
        auto secondColor = _parseColor("secondColor", node, style, ref->getSecondColor(), base); 
        auto flex = _parseFloat("flex", node, style, 0.0f, base);
        auto onUnhoverMethod = getListener(node.get("onUnhover").toString());
        auto margin = _parseDimensions("margin", node, style, nite::Vec2(0.0f), base);
        auto padding = _parseDimensions("padding", node, style, nite::Vec2(8.0f), base);
        auto backgroundImage = _parseString("backgroundImage", node, NULL, "", base);
        auto id = _parseString("id", node, style, base->literalId, base);
        auto font = _parseString("font", node, style, "", base); 
        // auto userShader = _parseShader("shader", node, style, base);
        _parseNav("navigate", node, base);
        _parseListenOn("listenOn", node, listeners, base);
        // if(userShader.success){
        //     base->apply(nite::Shader(userShader.frag, userShader.vert));
        // }         
        if(font != "" && nite::fileExists(font)){
            ref->setFont(nite::Font(font,  fontSize));
        }      
        auto snapInterpRate = _parseFloat("snapInterpRate", node, style, ref->snapInterpRate, base);
        ref->setSnapInterpRate(snapInterpRate);             
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
        ref->setOnClick(onClickMethod);
        if(node.has("flex") && flex > 0.0f){
            ref->setFlex(flex);
        }else{
            ref->setFlex(0.0f);   
            ref->fillUpType = false;   
            ref->setSize(size.size);
            ref->relSize.set(size.size);
            ref->useRelSizeX = size.isX;
            ref->useRelSizeY = size.isY;
        }  
        ref->setText(text);
        if(backgroundImage != "" && nite::fileExists(backgroundImage)){
            ref->setBackgroundImage(nite::Texture(backgroundImage));
        }        
    }else
    if(type == "icon"){
        base = Shared<nite::BaseUIComponent>(new nite::IconUI());
        auto *ref = static_cast<nite::IconUI*>(base.get());
        auto layout = _parseLayout(node, style, base);
        auto onClickMethod = getListener(node.get("onClick").toString());
        auto onHoverMethod = getListener(node.get("onHover").toString());    
        auto backgroundColor = _parseColor("backgroundColor", node, style, nite::Color(1.0f, 1.0f, 1.0f, 1.0f), base);     
        auto size = _parseSize(node, style, nite::Vec2(16.0f), base); 
        auto flex = _parseFloat("flex", node, style, ref->flex, base);
        auto onUnhoverMethod = getListener(node.get("onUnhover").toString());
        auto margin = _parseDimensions("margin", node, style, nite::Vec2(0.0f), base);
        auto padding = _parseDimensions("padding", node, style, nite::Vec2(0.0f), base);
        auto iconSize = _parseDimensions("iconSize", node, style, ref->getIconSize(), base);
        auto id = _parseString("id", node, style, base->literalId, base);
        auto source = _parseString("source", node, style, "", base);
        auto index = _parseInt("index", node, style, ref->getIndex(), base);
        auto textPosition = _parseDimensions("textPosition", node, style, ref->getTextPosition(), base);
        auto text = _parseString("text", node, style, ref->getText(), base);
        auto fontColor = _parseColor("fontColor", node, style, ref->getTextColor(), base); 
        auto fontSize = _parseInt("fontSize", node, style, ref->getFontSize(), base);
        auto font = _parseString("font", node, style, "", base); 
        auto shadowOffset = _parseDimensions("shadowOffset", node, style, ref->getShadowOffset(), base); 
        auto shadowColor = _parseColor("shadowColor", node, style, ref->getShadowColor(), base); 
        // auto userShader = _parseShader("shader", node, style, base);
        _parseNav("navigate", node, base);
        _parseListenOn("listenOn", node, listeners, base);
        // if(userShader.success){
        //     base->apply(nite::Shader(userShader.frag, userShader.vert));
        // }                 
        if(font != "" && nite::fileExists(font)){
            ref->setFont(nite::Font(font,  fontSize));
        }           
        auto snapInterpRate = _parseFloat("snapInterpRate", node, style, ref->snapInterpRate, base);
        ref->setSnapInterpRate(snapInterpRate);
        ref->setShadowColor(shadowColor);
        ref->setShadowOffset(shadowOffset);           
        ref->setFontSize(fontSize);    
        ref->setText(text);
        ref->setTextColor(fontColor);
        ref->setTextPosition(textPosition);
        ref->setId(id);    
        ref->setMargin(margin);
        ref->setPadding(padding);        
        ref->setOnUnhover(onUnhoverMethod);      
        ref->setIndex(index);
        ref->setIconSize(iconSize);
        if(node.has("flex") && flex > 0.0f){
            ref->setFlex(flex);
        }else{
            ref->setFlex(0.0f);   
            ref->fillUpType = false;   
            ref->setSize(size.size);
            ref->relSize.set(size.size);
            ref->useRelSizeX = size.isX;
            ref->useRelSizeY = size.isY;
        }  
        ref->setBackgroundColor(backgroundColor);
        ref->setOnClick(onClickMethod);
        ref->setOnHover(onHoverMethod);
        ref->setLayout(layout);
        if(source != "" && nite::fileExists(source)){
            ref->setSource(nite::Texture(source));
        }         
    }else    
    if(type == "panel"){
        base = Shared<nite::BaseUIComponent>(new nite::PanelUI());
        auto *ref = static_cast<nite::PanelUI*>(base.get());
        auto layout = _parseLayout(node, style, base);
        auto onClickMethod = getListener(node.get("onClick").toString());
        auto onHoverMethod = getListener(node.get("onHover").toString());    
        auto backgroundColor = _parseColor("backgroundColor", node, style, nite::Color(1.0f, 1.0f, 1.0f, 0.0f), base);     
        auto size = _parseSize(node, style, nite::Vec2(16.0f), base); 
        auto flex = _parseFloat("flex", node, style, ref->flex, base);
        auto borderColor = _parseColor("borderColor", node, NULL,ref->borderColor, base);
        auto borderthickness = _parseFloat("borderThickness", node, NULL, 0.0f, base);
        auto onUnhoverMethod = getListener(node.get("onUnhover").toString());
        auto margin = _parseDimensions("margin", node, style, nite::Vec2(0.0f), base);
        auto padding = _parseDimensions("padding", node, style, nite::Vec2(0.0f), base);
        auto id = _parseString("id", node, style, base->literalId, base);
        auto backgroundImage = _parseString("backgroundImage", node, NULL, "", base);
        // auto userShader = _parseShader("shader", node, style, base);
        auto clickAnalogue = _parseString("clickAnalogueKey", node, NULL, "W", base);
        auto snapInterpRate = _parseFloat("snapInterpRate", node, style, ref->snapInterpRate, base);
        ref->setSnapInterpRate(snapInterpRate);        
        _parseOverflow("overflow", node, base);
        _parseNav("navigate", node, base);
        _parseListenOn("listenOn", node, listeners, base);
        // if(userShader.success){
        //     nite::Shader shader;
        //     shader.load(userShader.frag, userShader.vert);
        //     base->apply(shader);
        // }         
        ref->setId(id);    
        ref->setMargin(margin);
        ref->setPadding(padding);        
        ref->setOnUnhover(onUnhoverMethod);   
        ref->onClickAnalogueKey = nite::translateKey(clickAnalogue);             
        if(node.has("flex") && flex > 0.0f){
            ref->setFlex(flex);
        }else{
            ref->setFlex(0.0f);   
            ref->fillUpType = false;   
            ref->setSize(size.size);
            ref->relSize.set(size.size);
            ref->useRelSizeX = size.isX;
            ref->useRelSizeY = size.isY;
        }        
        ref->setBackgroundColor(backgroundColor);
        ref->setOnClick(onClickMethod);
        ref->setOnHover(onHoverMethod);
        ref->setLayout(layout);
        ref->setBorderThickness(borderthickness);
        ref->setBorderColor(borderColor);        
        if(backgroundImage != "" && nite::fileExists(backgroundImage)){
            ref->setBackgroundImage(nite::Texture(backgroundImage));
        }         
    }else{
        nite::print("parsing component without/undefined type from a UI JSON");    
    }

    // parse children
    auto _c = node.get("children");
    for(int i = 0; i < _c.getCount(); ++i){
        auto child = _c.get(i);
        auto comp = nite::UI::build(child, styles, listeners, idsLookUp); 
        comp->nav.index = i;
        idsLookUp[comp->literalId] = comp;
        base->add(comp);
    } 

    return  base;
}

Shared<nite::BaseUIComponent> nite::UI::build(Jzon::Node &node, Dict<String, Jzon::Node> &styles){
    Dict<String, nite::ListenerLambda> dummy;
    return nite::UI::build(node, styles, dummy);
}

Shared<nite::BaseUIComponent> nite::UI::build(Jzon::Node &node){
    Dict<String, nite::ListenerLambda> dummy;
    Dict<String, Jzon::Node> styles;
    return nite::UI::build(node, styles, dummy);
}

Shared<nite::BaseUIComponent> nite::UI::build(Jzon::Node &node, Dict<String, Jzon::Node> &styles, Dict<String, nite::ListenerLambda> &listeners){
    Dict<String, Shared<nite::BaseUIComponent>> idsLookUp;
    return nite::UI::build(node, styles, listeners, idsLookUp);
}

// build from file

Shared<nite::BaseUIComponent> nite::UI::build(const String &path){
    Dict<String, nite::ListenerLambda> listeners;
    return build(path, listeners);
}

Shared<nite::BaseUIComponent> nite::UI::build(const String &path, Dict<String, nite::ListenerLambda> &listeners){
    Dict<String, Jzon::Node> styles;
    Jzon::Parser parser;
    Jzon::Node obj = parser.parseFile(path);
    JsonSource source;
    UInt64 timer = nite::getTicks();

    if(!obj.isValid()){
        nite::print("failed to build UI from JSON '"+path+"': "+parser.getError());
        return Shared<nite::BaseUIComponent>(NULL);
    }

    auto comp = nite::UI::build(obj, styles, listeners, source.idsLookUp);

    // only windows are automatically handled by the system itself
    // every other component is completely untethered
    if(comp.get() != NULL && comp->type == "window"){
        source.listeners = listeners;  
        source.path = path;
        source.hash = nite::hashFile(path);
        source.ui = comp;
        sources.push_back(source);  
        comp->idsLookUp = source.idsLookUp;
        nite::UI::add(comp);      
    }

    if(comp.get() == NULL){
        nite::print("failed to build UI from JSON '"+path+"'");
        return Shared<nite::BaseUIComponent>(NULL);
    }

    nite::print("built UI from JSON '"+path+"' | "+nite::toStr(nite::getTicks()-timer)+" msecs");

    return comp;
}

void nite::UI::update(){
    // no refresh for now
    // for(int i = 0; i < sources.size(); ++i){
    //     auto &so = sources[i];
    //     if(nite::getTicks()-so.lastTick > 1000){
    //         so.lastTick = nite::getTicks();
    //         auto hash = nite::hashFile(so.path);
    //         if(hash != so.hash){
    //             nite::print("changes on UI '"+so.path+"': reloading...");
    //             so.hash = hash;
    //             // _build(so.ui, so.path, so);
    //         }
    //     }
    // }
    for(int i = 0; i < components.size(); ++i){
        if(!components[i]->visible){
            continue;
        }
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