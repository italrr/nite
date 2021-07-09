#include "Object.hpp"

Game::SAT::Projection Game::SAT::project(Object *obj, const nite::Vec2 &axis){
    auto rotated = obj->mask.rotate(obj->orientation);
    float min =  axis.dotProduct(rotated.vert[0] + obj->position - obj->mask.origin * 0.5f);
    float max = min;
    for (int i = 1; i < obj->mask.vert.size(); i++) {
        // NOTE: the axis must be normalized to get accurate projections
        float p = axis.dotProduct(rotated.vert[i] + obj->position - obj->mask.origin * 0.5f);
        if(p < min){
            min = p;
        }else
        if(p > max){
            max = p;
        }
    }   
    return Projection(min, max);

}

Vector<nite::Vec2> Game::SAT::getAxes(Object *obj){
    auto rotated = obj->mask.rotate(obj->orientation);
    Vector<nite::Vec2> axes(obj->mask.vert.size(), 0);
    for (int i = 0; i < obj->mask.vert.size(); i++) {
        auto p1 = rotated.vert[i];
        auto p2 = rotated.vert[(i + 1) == rotated.vert.size() ? 0 : (i + 1)];
        auto edge = p1 - p2;
        auto normal = nite::Vec2(-edge.y, edge.x); 
        axes[i] = normal;
    }
    return axes;     
}


Game::Object::Object(){
    friction = 0.0f;
    setMass(0.0f);
    setPosition(nite::Vec2(0.0f));  
    // default shape - rectangle
    // 64x64
    auto sz = nite::Vec2(64.0f);
    auto shape = nite::Polygon();
    shape.rectangle(sz);
    reshape(shape, sz);
}

void Game::Object::setMass(float mass){
    this->mass = mass;
    this->invMass = mass <= 0 ? 0.0f : 1.0f / mass;
}


void Game::Object::reshape(const nite::Polygon &mask, const nite::Vec2 &size){
    this->orientation = 0.0f;
    this->mask = mask;
    this->size = size;
}

void Game::Object::push(float force, float angle){
    this->accel.x += (nite::cos(angle) * (force / 100.0f)) / this->mass;
    this->accel.y += (nite::sin(angle) * (force / 100.0f)) / this->mass;
}	

void Game::Object::move(float x, float y){
    // TODO: handle diagonal speed
    if(x != 0.0f){
        float diffx = ((x / 100.0f) / this->mass);
        if(nite::abs(this->accel.x) < nite::abs(diffx)){
            this->accel.x += diffx;
        }
    }
    if(y != 0.0f){
        float diffy = ((y / 100.0f) / this->mass);
        if(nite::abs(this->accel.y) < nite::abs(diffy)){
            this->accel.y += diffy;
        }
    }
}

void Game::Object::setPosition(const nite::Vec2 &p){
    this->position = p;
    this->lerpPos = p;
}