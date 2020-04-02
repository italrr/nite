#include <stdio.h>
#include <string.h>
#include <cmath>
#include <pthread.h>
#include <iostream>

#include "Console.hpp"
#include "Font.hpp"
#include "Texture.hpp"
#include "Graphics.hpp"
#include "Input.hpp"

static pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;

struct ProxyObject {
    String name;
    int type;
    size_t size;
    void *ref;
    bool serverSide;
    bool adminType;
    nite::Console::Function after;
    void set(const String &name, int type, size_t s, void *ref);
};

struct FunctionObject {
    nite::Console::Function function;
    bool adminType;
    bool serverSide;
};

struct BufferLine {
    String line;
    UInt64 time;
    nite::Color color;
};

struct Bind {
    unsigned key;
    String command;
    Bind(){

    }
    Bind(unsigned key, String command){
        this->key = key;
        this->command = command;
    }
};


static Vector<BufferLine> buffer;
static String userInput = "";
static Dict<String, ProxyObject> *proxies;
static Dict<String, FunctionObject> *functions;
static bool opened = false;
static bool showTime = false;
static int showLineNumber = 11;
static int showPredictionNumber = 5;
static int offset = 0;
static Vector<String> inputHistory;
static int currentHistorySelect;
static Vector<String> predictions;
static Dict<unsigned, Bind> binds;
static nite::Vec2 position;
static nite::Console::PipeFunction pipeServerSide = NULL;

static bool _init = false;
static void _preinit(){
    if(_init) return;
    // Zeroing maps 'proxies' and 'functions'
    proxies = new Dict<String, ProxyObject>();
    functions = new Dict<String, FunctionObject>();
    proxies->clear();
    functions->clear();
    binds.clear();
    _init = true;
}

/////////////
// COMMAND: bind
////////////
static nite::Console::Result cfBind(Vector<String> params){
    if(params.size() < 2){
        return nite::Console::Result("Not enough parameters(2)", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
    }
    String &_key = params[0];
    String command = params[1]; 
    // TODO: these two erases can cause the whole game to crash. improve this.
    while(command.at(0) == '"'){
        command.erase(0, 1);
    }
    while(command.at(command.size()-1) == '"'){
        command.erase(command.size()-1, command.size());
    }  
    int key = nite::translateKey(_key);
    if(key == -1){
        return nite::Console::Result("bind failed: '"+_key+"' is an unknown key", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
    }
    Bind bind;
    binds[key] = Bind(key, command); 
    return nite::Console::Result("bound '"+command+"' to keystroke '"+_key+"'", nite::Color(1.0f, 1.0f, 1.0f, 1.0f));
}
static auto cfBindIns = nite::Console::CreateFunction("bind", &cfBind);  

nite::Console::CreateProxy pShowLineNumber("cl_consoleln", nite::Console::ProxyType::Int, sizeof(int), &showLineNumber);
nite::Console::CreateProxy pOpened("console_op", nite::Console::ProxyType::Bool, sizeof(bool), &opened);
nite::Console::CreateProxy pShowPredictionNumber("cl_consolepreln", nite::Console::ProxyType::Int, sizeof(int), &showPredictionNumber);

void nite::Console::end(){
    proxies->clear();
    functions->clear();
    binds.clear();
    delete proxies;
    delete functions;  
}

bool nite::Console::createProxy(const String &name, int type, size_t s, void *ref, nite::Console::Function function, bool serverSide, bool adminType){
    ProxyObject proxy;
    proxy.name = name;
    proxy.type = type;
    proxy.size = s;
    proxy.adminType = adminType;
    proxy.ref = ref;
    proxy.serverSide = serverSide;
    proxy.after = function;
    _preinit();
    (*proxies)[name] = proxy;
    return true;
}

bool nite::Console::createFunction(const String &name, nite::Console::Function function, bool serverSide, bool adminType){
    _preinit();
    auto func = FunctionObject();
    func.function = function;
    func.serverSide = serverSide;
    func.adminType = adminType;
    (*functions)[name] = func;
    return true;
}

nite::Console::CreateProxy::CreateProxy(const String &name, int type, size_t s, void *ref){
    createProxy(name, type, s, ref, NULL, false, false);
}

nite::Console::CreateProxy::CreateProxy(const String &name, int type, size_t s, void *ref, nite::Console::Function function){
    createProxy(name, type, s, ref, function, false, false);
}

nite::Console::CreateFunction::CreateFunction(const String &name, nite::Console::Function function){
    createFunction(name, function, false, false);
}

nite::Console::CreateProxy::CreateProxy(const String &name, int type, size_t s, void *ref, bool serverSide, bool adminType){
    createProxy(name, type, s, ref, NULL, serverSide, adminType);
}

nite::Console::CreateProxy::CreateProxy(const String &name, int type, size_t s, void *ref, nite::Console::Function function, bool serverSide, bool adminType){
    createProxy(name, type, s, ref, function, serverSide, adminType);
}

nite::Console::CreateFunction::CreateFunction(const String &name, nite::Console::Function function, bool serverSide, bool adminType){
    createFunction(name, function, serverSide, adminType);
}

void nite::Console::pipeServerSideExecs(nite::Console::PipeFunction function){
    pipeServerSide = function;
    nite::print("set piping for server side execs on console");
}

void *nite::Console::getProxyReference(const String &name){
    if(proxies->count(name) == 0) return NULL;
    return (*proxies)[name].ref;
}

void nite::Console::add(const String &input, const nite::Color &color, bool print){
    if(input.length() == 0){
        return;
    }
    BufferLine line;
    line.line = input;
    line.time = nite::getTicks();
    line.color.set(color);
    buffer.push_back(line);
    if(print){
        pthread_mutex_lock(&count_mutex);
        String ts = nite::getTimestamp();
        std::cout << "[" << ts << "] " << input << std::endl;
        pthread_mutex_unlock(&count_mutex);    
    }
}

void nite::Console::add(const nite::Console::Result &result, bool print){
    add(result.msg, result.color, print);
}

void nite::Console::add(const String &input, bool print){
    add(input, nite::Color(1.0f, 1.0f, 1.0f, 1.0f), print);
}

void nite::Console::render(){
    static const int fs = 12 * nite::getGeneralScale();
    static nite::Texture tex("data/sprite/empty.png");
    static nite::Font font(nite::DefaultFontPath, fs, 1.5f);
    static const auto lh = font.getHeight() + 2;
    static const float cheight = lh * (showLineNumber + 1);
    static const nite::Vec2 offPosition(0.0f, -cheight);
    position.lerpAbsolute(opened ? nite::Vec2(0.0f) : offPosition, 0.42f);
    if(position.y == offPosition.y){
        return;
    }
    int j = (buffer.size() - showLineNumber) - offset;
    if(j < 0) j = 0;
    nite::setRenderTarget(nite::RenderTargetPosterioriEngine);
    nite::setDepth(nite::DepthTop);
    nite::setColor(0.0f, 0.0f, 0.0f, 0.98f);
    tex.draw(position.x, position.y, nite::getWidth(), cheight, 0.0f, 0.0f, 0.0f);
    nite::setColor(1.0f, 1.0f, 1.0f, 1.0f);
    int c = 0;
    int w = buffer.size() >= showLineNumber ? showLineNumber : buffer.size();
    for(int i = 0; i < w; ++i){
        nite::setColor(buffer[i + j].color);
        font.draw(buffer[i + j].line, position.x + 1.0f, position.y + 1.0f + lh * c++);
    }
    String tusr = "> "+userInput;
    nite::setColor(1.0f, 1.0f, 1.0f, 1.0f);
    font.draw(tusr, position.x + 1.0f, position.y + 1.0f + lh * showLineNumber);
    float tw = font.getWidth(tusr);
    static float pointerA = 100.0f;
    static bool pointerAC = true;
    if(nite::lerpAbsolute(pointerA, pointerAC ? 0.0f : 100.0f, 0.10f)){
        pointerAC = !pointerAC;
    }
    nite::setColor(1.0f, 1.0f, 1.0f, pointerA / 100.0f);
    tex.draw(position.x + 2.0f + tw, position.y + 0.0f + lh * showLineNumber, 1, font.getHeight() - 4.0f, 0.0f, 0.0f, 0.0f);
    if(predictions.size() > 0){
        nite::setColor(0.15f, 0.0f, 0.0f, 0.98f);
        auto *ref = tex.draw(position.x, position.y, nite::getWidth(), 0.0f, 0.0f, 0.0f, 0.0f);
        int c = 0;
        float mw = 0.0f;
        nite::setColor(1.0f, 1.0f, 1.0f, 1.0f);
        for(int i = 0; i < predictions.size(); ++i){
            ++c;
            font.draw(predictions[i], position.x + 1.0f, position.y + 1.0f + lh * (showLineNumber + 1 + i) );  
            mw = std::max(mw, font.getWidth(predictions[i]));
            if(c >= showPredictionNumber) break;
        }
        if(ref != NULL){
            ref->position.y = 1.0f + cheight;
            ref->size.y = lh * c;
            ref->size.x = mw + 4.0f;
        }
    }
}

static bool isProxy(const String &name){
    return proxies->count(name);
}

static bool isFunction(const String &name){
    return functions->count(name);
}

static bool isAnything(const String &name){
    return functions->count(name) > 0 || proxies->count(name) > 0;
}

static int queryInteger(ProxyObject &obj){
    return *((int*)obj.ref);
}

static float queryFloat(ProxyObject &obj){
    return *((float*)obj.ref);
}

static bool queryBool(ProxyObject &obj){
    return *((bool*)obj.ref);
}

static String queryString(ProxyObject &obj){
    return String((char*)obj.ref, obj.size);
}

static String queryLiteral(ProxyObject &obj){
    switch(obj.type){
        case nite::Console::ProxyType::Int:
            return nite::toStr(queryInteger(obj));
        case nite::Console::ProxyType::Float:
            return nite::toStr(queryFloat(obj));
        case nite::Console::ProxyType::Literal:
            return queryString(obj);            
        case nite::Console::ProxyType::Bool:
            return queryBool(obj) ? "true" : "false";
        default:
            return "[UNKOWN TYPE]";
    }
}

struct ModifyObjectState {
    String message;
    String value;
    bool success;
    ModifyObjectState(){
        success = false;
    }
};

static void modifyInteger(ProxyObject &obj, int val){
    memcpy(obj.ref, &val, sizeof(int));
}

static void modifyFloat(ProxyObject &obj, float val){
    memcpy(obj.ref, &val, sizeof(float));
}

static void modifyBool(ProxyObject &obj, bool val){
    memcpy(obj.ref, &val, sizeof(bool));
}

static void modifyString(ProxyObject &obj, String val){
    obj.size = val.size();
    memcpy(obj.ref, val.c_str(), val.size());
}


static ModifyObjectState modifyObject(ProxyObject &obj, int val){
    if(obj.type != nite::Console::ProxyType::Int){
        ModifyObjectState state;
        state.message = "Attempting to assign integer to a non-integer object";
        state.success = false;
        return state;
    }
    modifyInteger(obj, val);
    ModifyObjectState state;
    state.success = true;
    state.value = nite::toStr(val);
    return state;
}

static ModifyObjectState modifyObject(ProxyObject &obj, float val){
    if(obj.type != nite::Console::ProxyType::Float){
        ModifyObjectState state;
        state.message = "Attempting to assign float to a non-float object";
        state.success = false;
        return state;
    }
    modifyFloat(obj, val);
    ModifyObjectState state;
    state.success = true;
    state.value = nite::toStr(val);
    return state;
}

static ModifyObjectState modifyObject(ProxyObject &obj, bool val){
    if(obj.type != nite::Console::ProxyType::Bool){
        ModifyObjectState state;
        state.message = "Attempting to assign boolean to a non-boolean object";
        state.success = false;
        return state;
    }
    modifyBool(obj, val);
    ModifyObjectState state;
    state.success = true;
    state.value = val ? "true" : "false";
    return state;
}

static ModifyObjectState modifyObject(ProxyObject &obj, String val){
    if(obj.type != nite::Console::ProxyType::Literal){
        ModifyObjectState state;
        state.message = "Attempting to assign Literal to a non-literal object";
        state.success = false;
        return state;
    }
    modifyString(obj, val);
    ModifyObjectState state;
    state.success = true;
    state.value = val;
    return state;
}

static bool isBoolean(const String &val){
    return nite::toLower(val) == "true" || nite::toLower(val) == "false";
}

static bool isLiteral(const String &val){
    return val[0] == '"' && val[val.length()-1] == '"';
}

static UInt8 getOperandType(const String &operand){
    if(isBoolean(operand)){
        return nite::Console::ProxyType::Bool;
    }else
    if(nite::isNumber(operand)){
        return nite::Console::ProxyType::Float;
    }else
    if(isLiteral(operand)){
        return nite::Console::ProxyType::Literal;
    }
    return nite::Console::ProxyType::Unknown;
}

// TODO: this needs some improvement
static Vector<String> splitTokens(String input, char sep){
    Vector<String> tokens;
    bool quote = false;
    bool interp = false;
    for(int i = 0; i < input.size(); ++i){
        if(input[i] == '"'){
            quote = !quote;
        }

        if(!quote && input[i] == '{'){
            interp = true;
        } 

        if(!quote && input[i] == '}'){
            interp = false;
        }          
              
        if(input[i] == sep && !quote && !interp){
            tokens.push_back(input.substr(0, i));
            input = input.substr(i + 1, input.size() - i);
            i = -1;
        }
        if(i == input.size() - 1){
            tokens.push_back(input);
            break;
        }	
    }
    return tokens;
}

nite::Console::Result nite::Console::interpret(const String &command, bool remoteExec, bool svExec, bool asAdmin, bool silent){
    if(inputHistory.size() == 0 || inputHistory[inputHistory.size()-1] != command){
        inputHistory.push_back(command);
        currentHistorySelect = -1;
    }
    
    predictions.clear();
    static Vector<String> inputHistory;
    static int currentHistorySelect;
    
    auto tokens = splitTokens(command, ' ');
    if(tokens.size() == 0){
        auto r = nite::Console::Result("empty expression", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
        nite::Console::add(r);
        return r;
    }
    
    String imperative = tokens[0];

    // eval interporlations
    for(int i = 1; i < tokens.size(); ++i){
        if(!(tokens[i][0] == '{' && tokens[i][tokens[i].length()-1] == '}')) continue;
        tokens[i] = nite::Console::interpret(tokens[i].substr(1, tokens[i].length() - 2), false, false, false, true).msg;
    }
    
    String composed = imperative; // after interpolations

    for(int i = 1; i < tokens.size(); ++i){
        composed += " "+tokens[i];
    }


    // -- execute --
    // modify variable
    if(isProxy(imperative)){
        ProxyObject &op = (*proxies)[imperative];
        if(tokens.size() == 1){
            nite::Console::add((remoteExec ? "> " : "")+command, nite::Color(0.40f, 0.40f, 0.40f, 1.0f));
            nite::Console::Result r(queryLiteral(op), nite::Color(0.15f, 0.80f, 0.22f, 1.0f));
            nite::Console::add(r);
            return r;
        }else{
            String assignment = tokens[1];
            ModifyObjectState result;

            if(isLiteral(assignment)){
                assignment.erase(assignment.begin() + 0);
                assignment.erase(assignment.begin() + assignment.length()-1);
                result = modifyObject(op, assignment);
            }else
            if(nite::isNumber(assignment)){
                
                if(op.type == nite::Console::ProxyType::Int){
                    result = modifyObject(op, (int)nite::toInt(assignment));
                }else{
                    result = modifyObject(op, (float)nite::toFloat(assignment));
                }

            }else
            if(isBoolean(assignment)){
                result = modifyObject(op, assignment == "true");
            }else{
                nite::Console::Result r("'"+assignment+"' is an undefined symbol.", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
                nite::Console::add(r);
                return r;
            }

            if(result.success){
                nite::Console::Result r((remoteExec ? "> " : "")+op.name+" = "+result.value, nite::Color(0.0f, 0.9f, 0.0f, 1.0f));
                nite::Console::add(r);
                if(op.after != NULL)
                    op.after(Vector<String>());                
                return r;
            }else{
                nite::Console::Result r(result.message, nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
                nite::Console::add(r);
                return r;
            }
        }
    }else
    // Function
    if(isFunction(imperative)){
        Vector<String> params;
        for(int i = 1; i < tokens.size(); ++i){
            params.push_back(tokens[i]);
        }
        auto &func = (*functions)[imperative];
        nite::Console::Result r;
        if(func.serverSide && !svExec){
            if(pipeServerSide){
                pipeServerSide(composed);
                if(!silent){
                    nite::Console::add(command, nite::Color(0.40f, 0.40f, 0.40f, 1.0f));
                }
                return nite::Console::Result("noop", nite::Color(1.0f, 1.0f, 1.0f, 1.0f)); // technical limitation. server side interpolations resolve in noop
            }else{
                r = nite::Console::Result("you must be connected a server", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
                if(!silent){
                    nite::Console::add(r);
                }
                return r;
            }
        }else{
            nite::Console::Function function = func.function;
            if(func.adminType && !asAdmin){
                r = nite::Console::Result("you must be an admin to run this command", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
                if(!silent){
                    nite::Console::add(r);
                }
                return r;                
            }
            r = function(params);   
            if(!silent){
                nite::Console::add(r);     
            }
            return r;    
        }
    }
    if(!silent){
        nite::Console::add((remoteExec ? "> " : "")+command, nite::Color(0.40f, 0.40f, 0.40f, 1.0f));
    }
    nite::Console::Result r("unknown expression or undefined symbol", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
    if(!silent){
        nite::Console::add(r);
    }
    return r;    
}

static void buildPredections(){
    predictions.clear();	
    for(auto const& x : (*functions)){
        int i = x.first.find(userInput);
        if(i == 0){
            predictions.push_back(x.first);
        }
    }
    for(auto const& x : (*proxies)){
        int i = x.first.find(userInput);
        if(i == 0){
            predictions.push_back(x.first);
        }    
    }
}

void nite::Console::update(){
    for (auto& it : binds){
        auto &bind = binds[it.first];
        if(nite::keyboardPressed(bind.key)){
            nite::Console::interpret(bind.command, true);
        }
    }
    if(opened && nite::keyboardPress(nite::keyPAGEUP) && ((buffer.size() - showLineNumber) - offset) > 0 ){
        ++offset;
    }else
    if(opened && nite::keyboardPress(nite::keyPAGEDOWN) && offset > 0){
        --offset;
    }
    if(opened && nite::keyboardPressed(nite::keyENTER) && userInput.length() > 0){
        nite::Console::interpret(strRemoveEndline(userInput), false);
        userInput = "";
    }
    if(opened){
        String orig = userInput;
        userInput += nite::keyboardGetText();
        if(orig != userInput && userInput.length() > 0){
            buildPredections();
        }
    }
    if(opened && nite::keyboardPress(nite::keyBACK) && userInput.length() > 0){
        userInput.erase(userInput.length() - 1, 1);
        if(userInput.length() > 0){
            buildPredections();
        }
    }

    if(opened && nite::keyboardPressed(nite::keyTAB) && predictions.size() > 0){
        userInput = predictions[0];
        predictions.clear();
    }  

    if(opened && nite::keyboardPressed(nite::keyUP) && inputHistory.size() > 0){
        predictions.clear();
        ++currentHistorySelect;
        if(currentHistorySelect >= inputHistory.size()){
            currentHistorySelect = inputHistory.size() - 1;
        }
        userInput = inputHistory[inputHistory.size() - 1 - currentHistorySelect];
    }

    if(opened && nite::keyboardPressed(nite::keyDOWN) && inputHistory.size() > 0){
        predictions.clear();
        --currentHistorySelect;
        if(currentHistorySelect < 0){
            currentHistorySelect = 0;
        }
        userInput = inputHistory[inputHistory.size() - 1 - currentHistorySelect];
    }

}

void nite::Console::open(){
    predictions.clear();
    opened = true;
}

void nite::Console::close(){
    predictions.clear();
    opened = false;
}

static nite::Console::Result cfClose(Vector<String> params){
    nite::Console::close();
    return nite::Console::Result();
}

static auto cfCloseIns = nite::Console::CreateFunction("close", &cfClose);

bool nite::Console::isOpen(){
    return opened;
}