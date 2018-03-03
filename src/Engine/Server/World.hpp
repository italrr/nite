#ifndef WORLD_H
  #define WORLD_H
    #include "../Types.hpp"
    struct entityT;
    struct worldT {
      Vector<entityT*> objects;
      Vector<entityT*> activeObjects;
      nite::Vec2 dimensions;
      unsigned dimSize;
      worldT(int dimX, int dimY);
      worldT();
      void init(int dimX, int dimY);
      entityT *getObject(int objectId);
      entityT *getObject(int x, int y);
      bool isEmpty(int x, int y);
      bool isInWorld(entityT *obj);
      void add(entityT *obj, int x, int y);
      int getID(int x, int y);
      int getID(entityT *obj);
      void move(entityT *obj, int x, int y);
      void remove(entityT *obj);
      void remove(int x, int y);
      void removeActiveObject(entityT *obj);
      void addActiveObject(entityT *obj);
      void update();
    };
#endif
