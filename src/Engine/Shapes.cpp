#include "View.hpp"
#include "Tools/Tools.hpp"
#include "Shapes.hpp"
#define NO_SDL_GLEXT
#include "Shader.hpp"
#include "GLEW/GL/glew.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>


static void drawRectangle(nite::Renderable *object){
	nite::RenderableRectangle &obj = *(nite::RenderableRectangle*)object;
	glLineWidth(obj.thickness);
	glColor4f(obj.color.r, obj.color.g, obj.color.b, obj.color.a);
	nite::Vec2 offset = nite::getRenderOffset();
	glPushMatrix();
		glTranslatef(obj.position.x - nite::getViewX(obj.target) + 0.5f + offset.x, obj.position.y - nite::getViewY(obj.target) + 0.5f + offset.y, 0.f);
		if(obj.angle != 0) glRotatef(obj.angle, 0.0, 0.0, 1.0);
		for(int i = 0; i < obj.programs.size(); ++i){
			glUseProgram(obj.programs[i]->id);
			for(const auto& Uniform : obj.programs[i]->uniforms.integers){
				int tex = glGetUniformLocation(obj.programs[i]->id, Uniform.first.c_str());
				if(tex != -1){
					glUniform1i(tex, Uniform.second);
				}else{
					if(!obj.programs[i]->ref->faulty){
						nite::print("'"+obj.programs[i]->shaderName+"': Failed to find Shader Location '"+Uniform.first+"'");
						obj.programs[i]->ref->faulty = true;
					}
				}
			}
			for(const auto& Uniform : obj.programs[i]->uniforms.floats){
				int tex = glGetUniformLocation(obj.programs[i]->id, Uniform.first.c_str());
				if(tex != -1){
					glUniform1f(tex, Uniform.second);
				}else{
					if(!obj.programs[i]->ref->faulty){
						nite::print("'"+obj.programs[i]->shaderName+"': Failed to find Shader Location '"+Uniform.first+"'");
						obj.programs[i]->ref->faulty = true;
					}
				}
			}
			for(const auto& Uniform : obj.programs[i]->uniforms.vectors){
				int tex = glGetUniformLocation(obj.programs[i]->id, Uniform.first.c_str());
				if(tex != -1){
					float v[2];
					v[0] = Uniform.second.x;
					v[1] = Uniform.second.y;
					glUniform2fv(tex, 1, v);
				}else{
					if(!obj.programs[i]->ref->faulty){
						nite::print("'"+obj.programs[i]->shaderName+"': Failed to find Shader Location '"+Uniform.first+"'");
						obj.programs[i]->ref->faulty = true;
					}
				}
			}
		}
		GLfloat Box[] = {
			-obj.origin.x, -obj.origin.y,
			obj.size.x-obj.origin.x + 0.5f, -obj.origin.y,
			obj.size.x-obj.origin.x + 0.5f, obj.size.y-obj.origin.y + 0.5f,
			-obj.origin.x, obj.size.y-obj.origin.y + 0.5f,
		};
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(2, GL_FLOAT, 0, Box );
		glDrawArrays(obj.fill ? GL_QUADS : GL_LINE_LOOP, 0, 4);
		glDisableClientState(GL_VERTEX_ARRAY);
	glPopMatrix();
	glUseProgram(0);
	obj.programs.clear();
}

static void drawLine(nite::Renderable *object){
	nite::RenderableLine &obj = *(nite::RenderableLine*)object;
	nite::Vec2 offset = nite::getRenderOffset();	
	glColor4f(obj.color.r, obj.color.g, obj.color.b, obj.color.a);
	glLineWidth(obj.thickness);
	glPushMatrix();
		glTranslatef(obj.positionStart.x - nite::getViewX(obj.target) + 0.5f + offset.x,  obj.positionStart.y - nite::getViewY(obj.target) + 0.5f + offset.y, 0.f);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);		
		GLfloat Box[] = {0.0f, 0.0f,	obj.positionStart.x-obj.positionEnd.x, obj.positionStart.y -obj.positionEnd.y};
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(2, GL_FLOAT, 0, Box);
		glDrawArrays(GL_LINES, 0, 2);
		glDisableClientState(GL_VERTEX_ARRAY);
	glPopMatrix();
	obj.programs.clear();
}

static inline bool isClippingOut(float x, float y, float w, float h, float origX, float origY){
	nite::Vec2 size = nite::getSize() * 2.0f;
	nite::Vec2 v = nite::getView(nite::getRenderTarget()) - size / 4.0f;
	nite::Vec2 origCalc = nite::Vec2(w*origX, h*origY);
	nite::Vec2 p = nite::Vec2(x, y);
	nite::Vec2 s = nite::Vec2(w, h) - origCalc;
	return 	(p.x - s.x < v.x && p.x + s.x < v.x) || (p.x + s.x > v.x + size.x && p.x - s.x > v.x + size.x) ||
					(p.y - s.y < v.y && p.y + s.y < v.y) || (p.y + s.y > v.y + size.y && p.y - s.y > v.y + size.y);
	//return false;
}

nite::RenderableLine *nite::Draw::Line(float x, float y, float x2, float y2){
	nite::RenderableLine *obj = new nite::RenderableLine();
	obj->positionEnd.x = x2;
	obj->positionEnd.y = y2;
	obj->positionStart.x = x;
	obj->positionStart.y = y;
	obj->thickness = 1.0f;
	obj->function	= &drawLine;
	obj->color = nite::getColor();
	nite::addRenderList((nite::Renderable*)obj);
	return obj;
}

nite::RenderableRectangle *nite::Draw::Rectangle(float x, float y, float w, float h, bool fill, float origX, float origY, float angle){
	if(isClippingOut(x, y, w, h, origX, origY)) return NULL;
	nite::RenderableRectangle *obj = new nite::RenderableRectangle();
	obj->size.x			= w;
	obj->size.y 		= h;
	obj->position.x 	= x;
	obj->position.y 	= y;
	obj->thickness = 1.0f;
	obj->angle 			= angle;
	obj->fill			= fill;
	obj->origin			= nite::Vec2(origX*w, origY*h);
	obj->function		= &drawRectangle;
	obj->color			= nite::getColor();
	nite::addRenderList((nite::Renderable*)obj);
	return obj;
}

nite::RenderableRectangle *nite::Draw::Rectangle(float x, float y, float w, float h, bool fill){
	if(isClippingOut(x, y, w, h, 0.0f, 0.0f)) return NULL;
	nite::RenderableRectangle *obj = new nite::RenderableRectangle();
	obj->size.x			= w;
	obj->size.y 		= h;
	obj->position.x 	= x;
	obj->position.y 	= y;
	obj->angle 			= 0;
	obj->thickness = 1.0f;
	obj->fill			= fill;
	obj->origin			= nite::Vec2(0, 0);
	obj->function		= &drawRectangle;
	obj->color			= nite::getColor();
	nite::addRenderList((nite::Renderable*)obj);
	return obj;
}

nite::RenderableRectangle *nite::Draw::Rectangle(const nite::Vec2 &P, const nite::Vec2 &S, bool fill, const nite::Vec2 &orig, float angle){
	if(isClippingOut(P.x, P.y, S.x, S.y, orig.x, orig.y)) return NULL;
	nite::RenderableRectangle *obj = new nite::RenderableRectangle();
	obj->size.x			= S.x;
	obj->size.y 		= S.y;
	obj->position.x 	= P.x;
	obj->position.y 	= P.y;
	obj->angle 			= angle;
	obj->thickness 		= 1.0f;
	obj->fill			= fill;
	obj->origin			= nite::Vec2(orig.x*S.x, orig.y*S.y);
	obj->function		= &drawRectangle;
	obj->color			= nite::getColor();
	nite::addRenderList((nite::Renderable*)obj);
	return obj;
}

nite::RenderableRectangle *nite::Draw::Rectangle(const nite::Vec2 &P, const nite::Vec2 &S, bool fill){
	if(isClippingOut(P.x, P.y, S.x, S.y, 0.0f, 0.0f)) return NULL;
	nite::RenderableRectangle *obj = new nite::RenderableRectangle();
	obj->size.x			= S.x;
	obj->size.y 		= S.y;
	obj->position.x 	= P.x;
	obj->position.y 	= P.y;
	obj->angle 			= 0;
	obj->fill			= fill;
	obj->thickness 		= 1.0f;
	obj->origin			= nite::Vec2(0, 0);
	obj->function		= &drawRectangle;
	obj->color			= nite::getColor();
	nite::addRenderList((nite::Renderable*)obj);
	return obj;
}
