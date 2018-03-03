#include "World.hpp"
#include "Entity.hpp"
#include "../Tools/Tools.hpp"

using namespace nite;

worldT::worldT(){
  dimSize = 0;
}

void worldT::init(int dimX, int dimY){
  dimensions.set(dimX, dimY);
  dimSize = dimX*dimY;
  for(unsigned i=0; i<dimSize; ++i){
    objects.push_back(NULL);
  }
}

worldT::worldT(int dimX, int dimY){
  init(dimX, dimY);
}

entityT *worldT::getObject(int objectId){
    return objectId >= dimSize || objectId<0 ? NULL : objects[objectId];
}

entityT *worldT::getObject(int x, int y){
  int objectId = getID(x, y);
  return objectId >= dimSize || objectId<0 ? NULL : objects[objectId];
}

bool worldT::isEmpty(int x, int y){
  return getObject(x, y) == NULL;
}

bool worldT::isInWorld(entityT *obj){
  return getID(obj) != -1;
}

void worldT::add(entityT *obj, int x, int y){
  if(obj == NULL){
    print("NULL object.");
    return;
  }
  if(isInWorld(obj)){
    print("This object is already in the world. move it.");
    return;
  }
  if(!isEmpty(x, y)){
    print("Space occupied.");
    return;
  }
  int objectId = getID(x, y);
  objects[objectId] = obj;
  obj->create();
  obj->World = this;
  obj->position.set(x, y);
  addActiveObject(obj);
}

void worldT::addActiveObject(entityT *obj){
  activeObjects.push_back(obj);
}

void worldT::removeActiveObject(entityT *obj){
  for(unsigned i=0; i<activeObjects.size(); ++i){
    if(activeObjects[i] == obj){
      activeObjects.erase(activeObjects.begin()+i);
      return;
    }
  }
}

int worldT::getID(int x, int y){
  return x + y*dimensions.x;
}

int worldT::getID(entityT *obj){
  for(unsigned i=0; i<objects.size(); ++i){
    if(objects[i] == obj) return i;
  }
  return -1;
}

void worldT::remove(entityT *obj){
  if(obj == NULL){
    print("Can't remove a NULL object.");
    return;
  }
  if(!isInWorld(obj)){
    print("object is not part of the world.");
    return;
  }
  remove(obj->position.x, obj->position.y);
  removeActiveObject(obj);
}

void worldT::update(){
  for(unsigned i=0; i<activeObjects.size(); ++i){
    activeObjects[i]->update();
  }
}

void worldT::move(entityT *obj, int x, int y){ // Quick move, no security checks!
  //Invalid Coordinates
  if(x < 0 || x > dimensions.x || y < 0 || y > dimensions.y){
    return;
  }
  if(!isEmpty(x, y)){
    return;
  }
  int oID = getID(obj->position.x, obj->position.y);
  int nID = getID(x, y);
  objects[nID] = objects[oID];
  objects[oID] = NULL;
  objects[nID]->position.set(x, y);
}

void worldT::remove(int x, int y){
  int objectId = getID(x, y);
  if(objectId < 0 || objectId > dimSize){
    print("object is not part of the world.");
    return;
  }
  removeActiveObject(objects[objectId]);
  objects[objectId]->destroy();
  objects[objectId]->World = NULL;
  objects[objectId] = NULL;
}
