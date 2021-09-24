#include "Engine/Input.hpp"
#include "Storyline.hpp"

static int indCounter = nite::randomInt(1000, 2000);

Game::Story::Identifier::Identifier(){
    refId = ++indCounter;
    static hashidsxx::Hashids hash("STORY LINE OBJECT IDENTIFIER SALT");
    symRefId = hash.encode({refId, 8493, 1004});
}

/*
        MEMORY OBJECT
*/
Game::Story::MemoryObject::MemoryObject(){
    data = NULL;
    dataType = MemoryObjectDataType::NONE;
}

Game::Story::MemoryObject::~MemoryObject(){
    clear();
}

void Game::Story::MemoryObject::clear(){
    if(data == NULL){
        return;
    }
    delete data;
    data = NULL;
}

bool Game::Story::MemoryObject::read(void *target, size_t bytes){
    if(data == NULL){
        return false;
    }
    memcpy(target, data, bytes);
    return 
    true;
}

bool Game::Story::MemoryObject::write(const void *source, size_t bytes, int type){
    if(data == NULL){
        data = static_cast<char*>(malloc(bytes));        
    }    
    size = bytes;
    dataType = type;
    memcpy(data, source, bytes);
    return true;                    
}

/*
        MEMORY OBJECT [INTEGER]
*/
Game::Story::MemoryObjectInt::MemoryObjectInt(){
    dataType = MemoryObjectDataType::INTEGER;
    data = static_cast<char*>(malloc(sizeof(int32_t)));
} 

bool Game::Story::MemoryObjectInt::write(int32_t v){
    return write(static_cast<void*>(&v), sizeof(int32_t), MemoryObjectDataType::INTEGER);
}

bool Game::Story::MemoryObjectInt::read(int32_t &v){
    return read(&v, sizeof(int32_t));
}  


/*
        MEMORY OBJECT [FLOAT]
*/
Game::Story::MemoryObjectFloat::MemoryObjectFloat(){
    dataType = MemoryObjectDataType::FLOAT;
    data = static_cast<char*>(malloc(sizeof(float)));
}

bool Game::Story::MemoryObjectFloat::write(float v){
    return write(static_cast<void*>(&v), sizeof(float), MemoryObjectDataType::FLOAT);
}

bool Game::Story::MemoryObjectFloat::read(float &v){
    return read(&v, sizeof(float));
}

/*
        MEMORY OBJECT [BOOL]
*/
Game::Story::MemoryObjectBool::MemoryObjectBool(){
    dataType = MemoryObjectDataType::BOOLEAN;
    data = static_cast<char*>(malloc(sizeof(bool)));
}

bool Game::Story::MemoryObjectBool::write(bool v){
    return write(static_cast<void*>(&v), sizeof(bool), MemoryObjectDataType::BOOLEAN);
}

bool Game::Story::MemoryObjectBool::read(bool &v){
    return read(&v, sizeof(bool));
}

Game::Story::MemoryObjectString::MemoryObjectString(){
    data = NULL;
    dataType = MemoryObjectDataType::STRING;
}

bool Game::Story::MemoryObjectString::write(const String &v){
    return write(static_cast<const void*>(v.data()), v.length(), MemoryObjectDataType::STRING);
}

bool Game::Story::MemoryObjectString::read(String &v){
    if(this->data == NULL){
        return false;
    }
    v = String(this->data, size);
    return true;
}


/*
        MemoryBox
*/

Game::Story::MemoryBox::MemoryBox(){

}

void Game::Story::MemoryBox::clearShortTerm(){
    this->shortTerm.clear();
}

Shared<Game::Story::MemoryObject> Game::Story::MemoryBox::get(const String &name){
    auto it = shortTerm.find(name);
    if(it == shortTerm.end()){
        auto it = bank.find(name);
        if(it == bank.end()){
            return Shared<Game::Story::MemoryObject>(NULL);
        }
        return it->second;
    }
    return it->second;
}

bool Game::Story::MemoryBox::store(const String &name, const String &v, bool tmp){
    auto object = Shared<Game::Story::MemoryObjectString>(new Game::Story::MemoryObjectString());
    object->write(v);
    auto &bank = tmp ? this->shortTerm : this->bank;
    bank[name] = object;
    return true;
}

bool Game::Story::MemoryBox::store(const String &name, int32_t v, bool tmp){
    auto object = Shared<Game::Story::MemoryObjectInt>(new Game::Story::MemoryObjectInt());
    object->write(v);
    auto &bank = tmp ? this->shortTerm : this->bank;
    bank[name] = object;
    return true;
}

bool Game::Story::MemoryBox::store(const String &name, float v, bool tmp){
    auto object = Shared<Game::Story::MemoryObjectFloat>(new Game::Story::MemoryObjectFloat());
    object->write(v);
    auto &bank = tmp ? this->shortTerm : this->bank;
    bank[name] = object;
    return true;
}

bool Game::Story::MemoryBox::store(const String &name, bool v, bool tmp){
    auto object = Shared<Game::Story::MemoryObjectBool>(new Game::Story::MemoryObjectBool());
    object->write(v);
    auto &bank = tmp ? this->shortTerm : this->bank;
    bank[name] = object;
    return true;   
}


/*
        InteractionTree
*/

void Game::Story::InteractionTree::run(){
    device->next("");
}

Game::Story::InteractionTree::InteractionTree(){
    interType = InteractionTreeType::NONE;
}  


/*
        InteractionTreeContact
*/

void Game::Story::InteractionTreeContact::run(){
    // depending on conditions, we run the correspding interaction
    // if no conditions, then it goes for the first one
    // TODO: check conditions
    if(conditions.size() == 0){
        if(next.size() == 0){
            this->device->end();
        }else{
            this->device->next(next[0]);
        }
    }
}

Game::Story::InteractionTreeContact::InteractionTreeContact(){
    interType = InteractionTreeType::CONTACT;
}  


/*
        InteractionTreeExposition
*/

void Game::Story::InteractionTreeExposition::run(){
    dialogDevice->reset();
    for(int i = 0; i < lines.size(); ++i){
        dialogDevice->add(lines[i]->emitter, lines[i]->message, lines[i]->color);
    }
    dialogDevice->onEndCallback = [&](){
        // TODO: check conditions
        if(next.size() == 0){
            this->device->end();
        }else{
            this->device->next(next[0]);
        }       
    };
    dialogDevice->start(nite::Vec2(0.0f), 720, 3);
    
}

Game::Story::InteractionTreeExposition::InteractionTreeExposition(){
    interType = InteractionTreeType::EXPOSITION;
}  


/*
        InteractionTreeDialog
*/

void Game::Story::InteractionTreeDialog::run(){
    optionsDevice->clear();
    for(int i = 0; i < options.size(); ++i){
        auto &opt = options[i];
        optionsDevice->addOption(options[i]->label, [&, opt](Game::UIListMenuOption *selOpt){
            switch(opt->type){
                case InteractionTreeDialogOptionType::NEXT: {
                    this->device->next(opt->next);
                    optionsDevice->setVisible(false);
                } break;
                case InteractionTreeDialogOptionType::SCRIPT: {
                    // TODO: implement script
                } break;                
                case InteractionTreeDialogOptionType::CALLBACK: {
                    opt->callback(this->symRefId);
                    optionsDevice->setVisible(false);
                } break;                
            }            
        });
    }
    optionsDevice->setTitle(true, title);
    optionsDevice->setSize(nite::Vec2(200.0f, 0.0f));    
    optionsDevice->setPosition(nite::Vec2(nite::getWidth() * 0.5f - optionsDevice->size.x * 0.5f, nite::getHeight() -  optionsDevice->size.y - optionsDevice->margin.y));                   
    // optionsDevice->setPosition(nite::Vec2(0.0f));
    optionsDevice->setVisible(true);
}

Game::Story::InteractionTreeDialog::InteractionTreeDialog(){
    interType = InteractionTreeType::DIALOG;
}  


/*
        InteractionDevice
*/

Game::Story::InteractionDevice::InteractionDevice(){
    busy = false;
    lastInter = nite::getTicks();
}

void Game::Story::InteractionDevice::addInter(Shared<InteractionTree> inter){
    inter->device = this;
    switch(inter->interType){
        case InteractionTreeType::DIALOG: {
            std::dynamic_pointer_cast<InteractionTreeDialog>(inter)->optionsDevice = optionsDevice;
        } break;
        case InteractionTreeType::EXPOSITION: {
            std::dynamic_pointer_cast<InteractionTreeExposition>(inter)->dialogDevice = dialogDevice;
        } break;
    }
    this->interactions[inter->symRefId] = inter;
}

void Game::Story::InteractionDevice::next(const String &symRefId){
    nite::print("[debug] interactions device: started node '"+symRefId+"'");
    auto iter = interactions.find(symRefId);
    if(iter == interactions.end()){
        nite::print("InteractionDevice: fatal failure: jumping to unexinsting node '"+symRefId+"': broken interaction");
        end();
        return;
    }
    iter->second->run();
}

void Game::Story::InteractionDevice::render(){

}

void Game::Story::InteractionDevice::step(){
    if(!busy){
        return;
    }
    if(nite::keyboardPressed(nite::keyZ)){
        this->dialogDevice->cont();
    }
}

void Game::Story::InteractionDevice::end(){
    nite::print("interaction end");
    busy = false;
    lastInter = nite::getTicks();
    dialogDevice->reset();
    optionsDevice->clear();
    optionsDevice->setVisible(false);
}

void Game::Story::InteractionDevice::start(const String &startInter){
    if(nite::getTicks()-lastInter < 100){
        nite::print("[debug] interaction delay");
        return;
    }
    this->startInter = startInter;
    if(this->startInter == ""){
        // if startInter is not provided, we look for a "contact" type
        // first one found, first one started
        for(auto &it : interactions){            
            if(it.second->interType == InteractionTreeType::CONTACT){
                this->startInter = it.second->symRefId;
                break;
            }
        }
    }
    if(this->startInter == ""){
        nite::print("InteractionDevice: fatal failure: current interaction does not have a contact node");
        return;
    }
    next(this->startInter);
    busy = true;
}


/*
        StoryLine
*/


Game::Story::StoryLine::StoryLine(){
    memBox = Shared<Game::Story::MemoryBox>(new Game::Story::MemoryBox());
    interDevice = std::make_shared<Game::Story::InteractionDevice>(Game::Story::InteractionDevice());
    interDevice->storyLine = this;
}

void Game::Story::StoryLine::loadStoryLine(const String &file){

}

void Game::Story::StoryLine::step(){
    interDevice->step();
}

void Game::Story::StoryLine::render(){
    interDevice->render();
}

void Game::Story::StoryLine::setup(Shared<UIListMenu> &optionsDevice, Shared<DialogBox> &dialogDevice){
    this->interDevice->optionsDevice = optionsDevice;
    this->interDevice->dialogDevice = dialogDevice;
}