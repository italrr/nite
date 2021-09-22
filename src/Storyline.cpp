#include "Storyline.hpp"

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