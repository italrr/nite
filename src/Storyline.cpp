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
}

Game::Story::InteractionTreeDialog::InteractionTreeDialog(){
    interType = InteractionTreeType::DIALOG;
}  


/*
        InteractionDevice
*/

Game::Story::InteractionDevice::InteractionDevice(){
    busy = false;
}

void Game::Story::InteractionDevice::addInter(Shared<InteractionTree> inter){
    auto copy = inter;
    copy->device = this;
    switch(inter->interType){
        case InteractionTreeType::DIALOG: {
            std::dynamic_pointer_cast<InteractionTreeDialog>(inter)->optionsDevice = optionsDevice;
        };
        case InteractionTreeType::EXPOSITION: {
            std::dynamic_pointer_cast<InteractionTreeExposition>(inter)->dialogDevice = dialogDevice;
        };
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
    dialogDevice->reset();
    optionsDevice->clear();
    optionsDevice->setVisible(false);
}

void Game::Story::InteractionDevice::start(const String &startInter){
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
    next(this->startInter);
    busy = true;
}