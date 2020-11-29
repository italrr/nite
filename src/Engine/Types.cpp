#include "Types.hpp"
#include "Tools/Tools.hpp"
#include <stdio.h>
#include <cstdlib>
#include <string>

using namespace nite;

/*
================
color
================
*/
nite::Color::Color(){
	r = 0;
	g	= 0;
	b	= 0;
	a	= 1.0f;
	rectify();
}

nite::Color::Color(const nite::Color &c){
	r = c.r;
	g	= c.g;
	b	= c.b;
	a	= c.a;
	rectify();
}


nite::Color::Color(float r, float g, float b){
	this->r = r;
	this->g = g;
	this->b = b;
	this->a	= 1.0f;
	rectify();

}

nite::Color::Color(float r, float g, float b, float a){
	this->r = r;
	this->g = g;
	this->b = b;
	this->a = a;
	rectify();
}

nite::Color::Color(const String &hex){
	set(hex);
}

void nite::Color::set(const String &hex){
	char *p;
	int hexValue = strtol(hex.substr(1).c_str(), &p, 16);
  r = ((hexValue >> 16) & 0xFF) / 255.0;  // Extract the RR byte
  g = ((hexValue >> 8) & 0xFF) / 255.0;   // Extract the GG byte
  b = ((hexValue) & 0xFF) / 255.0;        // Extract the BB byte
  a = 1.0f;
}

String nite::Color::hex(){
	int ri = r * 255.0f; int gi = g * 255.0f; int bi = b * 255.0f;
	int hex = ((ri & 0xff) << 16) + ((gi & 0xff) << 8) + (bi & 0xff); // loses alpha though
	char hexString[4*sizeof(int)+1];
	return String(hexString);
}

void nite::Color::set(float r, float g, float b, float a){
	this->r = r;
	this->g = g;
	this->b = b;
	this->a = a;
	rectify();
}

void nite::Color::set(const nite::Color &color){
	this->r = color.r;
	this->g	= color.g;
	this->b	= color.b;
	this->a	= color.a;
	rectify();
}

nite::Color::Color(float c){
	set(c);
}

nite::Color::Color(float c, float a){
	set(c, a);
}

void nite::Color::set(float c, float a){
	this->r = c;
	this->g	= c;
	this->b	= c;
	this->a	= a;
	rectify();
}

void nite::Color::set(float c){
	this->r = c;
	this->g	= c;
	this->b	= c;
	this->a	= c;
	rectify();	
}

nite::Color::operator String(){
	return str();
}

String nite::Color::str(){
	return "("+toStr(r)+", "+toStr(g)+", "+toStr(b)+", "+toStr(a)+")";
}

void nite::Color::to100(){
	r *= 100.0f;	
	g *= 100.0f;
	b *= 100.0f;
	a *= 100.0f;
}

void nite::Color::to1(){
	r /= 100.0f;	
	g /= 100.0f;
	b /= 100.0f;
	a /= 100.0f;
}

bool nite::Color::cInterpDiscrete(const Color &f, float Step){
	if(*this == f) return true;
	auto copy = f;
	copy.to100();
	this->to100();
	nite::cInterpDiscrete(r, copy.r, Step);
	nite::cInterpDiscrete(g, copy.g, Step);
	nite::cInterpDiscrete(b, copy.b, Step);
	nite::cInterpDiscrete(a, copy.a, Step);
	this->to1();	
	rectify();
	return false;
}

bool nite::Color::cInterp(const Color &f, float Step){
	if(*this == f) return true;
	auto copy = f;
	copy.to100();
	this->to100();
	nite::cInterp(r, copy.r, Step);
	nite::cInterp(g, copy.g, Step);
	nite::cInterp(b, copy.b, Step);
	nite::cInterp(a, copy.a, Step);
	this->to1();	
	rectify();
	return false;
}

void nite::Color::rectify(){
	if(r > 1.0f) r = 1.0f;
	if(r < 0.0f) r = 0.0f;
	
	if(g > 1.0f) g = 1.0f;
	if(g < 0.0f) g = 0.0f;

	if(b > 1.0f) b = 1.0f;
	if(b < 0.0f) b = 0.0f;
	
	if(a > 1.0f) a = 1.0f;
	if(a < 0.0f) a = 0.0f;			
}

void nite::Color::operator=(float c){
	r = c;
	g = c;
	b = c;
	a = c;
	rectify();
}

bool nite::Color::operator<(const nite::Color &v){
	return r < v.r && g < v.g && b < v.b && a < v.a;
}

bool nite::Color::operator>(const nite::Color &v){
	return r > v.r && g > v.g && b > v.b && a > v.a;
}

bool nite::Color::operator<=(const nite::Color &v){
	return r <= v.r && g <= v.g && b <= v.b && a <= v.a;
}

bool nite::Color::operator>=(const nite::Color &v){
	return r >= v.r && g >= v.g && b >= v.b && a >= v.a;
}

bool nite::Color::operator<(float c){
	return r < c && g < c && b < c && a < c;
}

bool nite::Color::operator>(float c){
	return r > c && g > c && b > c && a > c;
}

bool nite::Color::operator<=(float c){
	return r <= c && g <= c && b <= c && a <= c;
}

bool nite::Color::operator>=(float c){
	return r >= c && g >= c && b >= c && a >= c;
}

bool nite::Color::operator==(float c){
	return r == c && g == c && b == c && a == c;
}

bool nite::Color::operator==(const nite::Color &v){
	return r == v.r && g == v.g && b == v.b && a == v.a;
}

bool nite::Color::operator!=(float c){
	return r != c && g != c && b != c && a != c;
}

bool nite::Color::operator!=(const Color &v){
	return r != v.r && g != v.g && b != v.b && a != v.a;
}

nite::Color nite::Color::operator/(float c){
	auto b = *this;
	b.r /= c;
	b.g /= c;
	b.b /= c;
	b.rectify();
	return b;
}

nite::Color nite::Color::operator/(const nite::Color &v){
	auto b = *this;
	b.r /= v.r;
	b.g /= v.g;
	b.b /= v.b;
	b.rectify();
	return b;
}

nite::Color nite::Color::operator*(float c){
	auto b = *this;
	b.r *= c;
	b.g *= c;
	b.b *= c;
	b.rectify();
	return b;
}

nite::Color nite::Color::operator*(const nite::Color &v){
	auto b = *this;
	b.r *= v.r;
	b.g *= v.g;
	b.b *= v.b;
	b.rectify();
	return b;
}

nite::Color nite::Color::operator+(float c){
	auto b = *this;
	b.r += c;
	b.g += c;
	b.b += c;
	b.rectify();
	return b;
}

nite::Color nite::Color::operator+(const nite::Color &v){
	auto b = *this;
	b.r += v.r;
	b.g += v.g;
	b.b += v.b;
	b.rectify();
	return b;
}

nite::Color nite::Color::operator-(float c){
	auto b = *this;
	b.r -= c;
	b.g -= c;
	b.b -= c;
	b.rectify();
	return b;
}

nite::Color nite::Color::operator-(const nite::Color &v){
	auto b = *this;
	b.r -= v.r;
	b.g -= v.g;
	b.b -= v.b;
	b.rectify();
	return b;
}

nite::Color nite::Color::invert(){
	auto col = nite::Color();
	col.r = 1.0 - r;
	col.g = 1.0 - g;
	col.b = 1.0 - b;
	return col;
}

/*
================
Vector 2D
================
*/

bool nite::Vec2::operator<(float c){
	return x < c && y < c;
}

bool nite::Vec2::operator>(float c){
	return x > c && y > c;
}

bool nite::Vec2::operator<=(float c){
	return x <= c && y <= c;
}

bool nite::Vec2::operator>=(float c){
	return x >= c && y >= c;
}

bool nite::Vec2::operator!=(float c){
	return x != c || y != c;
}

bool nite::Vec2::operator!=(const Vec2 &v){
	return v.x != x || v.y != y;
}

void nite::Vec2::set(const Vec2 &v){
	x = v.x;
	y = v.y;
}
void nite::Vec2::set(float X, float Y){
	x = X;
	y = Y;
}

nite::Vec2::Vec2(){
	x = 0;
	y = 0;
}

nite::Vec2::Vec2(float X, float Y){
	x = X;
	y = Y;
}

nite::Vec2::Vec2(const Vec2 &Vec){
	x = Vec.x;
	y = Vec.y;
}

nite::Vec2::Vec2(float c){
	x = c;
	y = c;
}

bool nite::Vec2::operator<(const nite::Vec2 &v){
	return x < v.x && y < v.y;
}

bool nite::Vec2::operator>(const nite::Vec2 &v){
	return x > v.x && y > v.y;
}

bool nite::Vec2::operator<=(const nite::Vec2 &v){
	return x <= v.x && y <= v.y;
}

bool nite::Vec2::operator>=(const nite::Vec2 &v){
	return x >= v.x && y >= v.y;
}

nite::Vec2 nite::Vec2::operator/(float c){
	Vec2 R = *this;
	if (c == 0) R;
	R.x /= c;
	R.y /= c;
	return R;
}

nite::Vec2 nite::Vec2::operator/(const nite::Vec2 &v){
	Vec2 R = *this;
	if (v.x == 0 || v.y ==0) R;
	R.x /= v.x;
	R.y /= v.y;
	return R;
}

void nite::Vec2::operator=(float c){
	x = c;
	y = c;
}

nite::Vec2 nite::Vec2::operator*(float c){
	Vec2 R = *this;
	R.x *= c;
	R.y *= c;
	return R;
}

nite::Vec2 nite::Vec2::operator*(const nite::Vec2 &v){
	Vec2 R = *this;
	R.x *= v.x;
	R.y *= v.y;
	return R;
}


nite::Vec2 nite::Vec2::operator+(float c){
	Vec2 R = *this;
	R.x += c;
	R.y += c;
	return R;
}

nite::Vec2 nite::Vec2::operator+(const nite::Vec2 &v){
	Vec2 R = *this;
	R.x += v.x;
	R.y += v.y;
	return R;
}

nite::Vec2 nite::Vec2::operator-(float c){
	Vec2 R = *this;
	R.x -= c;
	R.y -= c;
	return R;
}

nite::Vec2 nite::Vec2::operator-(const nite::Vec2 &v){
	Vec2 R = *this;
	R.x -= v.x;
	R.y -= v.y;
	return R;
}

bool nite::Vec2::operator==(float c){
	return x == c && y == c;
}

bool nite::Vec2::operator==(const nite::Vec2 &v){
	return x == v.x && y == v.y;
}

bool nite::Vec2::cInterp(const Vec2 &v, float step){
	if (x == v.x && y == v.y) return true;
	nite::cInterp(x, v.x, step);
	nite::cInterp(y, v.y, step);
	return false;
}

bool nite::Vec2::cInterpDiscrete(const Vec2 &v, float step){
	if (x == v.x && y == v.y) return true;
	nite::cInterpDiscrete(x, v.x, step);
	nite::cInterpDiscrete(y, v.y, step);
	return false;
}

bool nite::Vec2::cInterpAbsolute(const Vec2 &v, float step){
	if (x == v.x && y == v.y) return true;
	nite::cInterpAbsolute(x, v.x, step);
	nite::cInterpAbsolute(y, v.y, step);
	return false;
}

bool nite::Vec2::lerpDiscrete(const Vec2 &v, float step, float limit){
	auto a = nite::lerpDiscrete(x, v.x, step, limit);
	auto b = nite::lerpDiscrete(y, v.y, step, limit);
	return a && b;
}

float nite::Vec2::avg(){
	return (x + y) * 0.5f;
}

nite::Vec2::operator String(){
	return str();
}

String nite::Vec2::str(){
	return "("+nite::toStr(x)+", "+nite::toStr(y)+")";
}


/*
================
Rectangle
================
*/
nite::Rect::Rect(){
	x = 0;
	y = 0;
	w = 0;
	h = 0;
}

nite::Rect::Rect(float X, float Y, float W, float H){
	set(X, Y, W, H);
}

void nite::Rect::set(const Vec2 &P, const Vec2 &Q){
	x = P.x;
	y = P.y;
	w = Q.x;
	h = Q.y;
}

nite::Rect::Rect(float x){
	set(x);
}


nite::Rect::Rect(const Rect &r){
	set(r);
}

void nite::Rect::set(const Rect &r){
	x = r.x;
	y = r.y;
	w = r.w;
	h = r.h;
}

void nite::Rect::set(float X, float Y, float W, float H){
	x = X;
	y = Y;
	w = W;
	h = H;
}

void nite::Rect::set(float c){
	x = c;
	y = c;
	w = c;
	h = c;
}

bool nite::Rect::operator<(float c){
	return x < c && y < c && w < c && h < c;
}

bool nite::Rect::operator>(float c){
	return x > c && y > c && w > c && h > c;
}

bool nite::Rect::operator<=(float c){
	return x <= c && y <= c && w <= c && h <= c;
}

bool nite::Rect::operator>=(float c){
	return x >= c && y >= c && w >= c && h >= c;
}

bool nite::Rect::operator!=(float c){
	return !(x == c && y == c && w == c && c == h);
}

bool nite::Rect::operator!=(const Rect &r){
	return !(r.x == x && r.y == y && r.w == w && r.h == h);
}

bool nite::Rect::operator<(const nite::Rect &r){
	return x < r.x && y < r.y && w < r.w && h < r.h;
}

bool nite::Rect::operator>(const nite::Rect &r){
	return x > r.x && y > r.y && w > r.w && h > r.h;
}

bool nite::Rect::operator<=(const nite::Rect &r){
	return x <= r.x && y <= r.y && w <= r.w && h <= r.h;
}

bool nite::Rect::operator>=(const nite::Rect &r){
	return x >= r.x && y >= r.y && w >= r.w && h >= r.h;
}

nite::Rect nite::Rect::operator/(float c){
	Rect R = *this;
	if (c == 0) R;
	R.x /= c;
	R.y /= c;
	R.w /= c;
	R.h /= c;
	return R;
}

nite::Rect nite::Rect::operator/(const nite::Rect &r){
	Rect R = *this;
	if (r.x == 0 || r.y ==0 || r.w == 0 || r.h == 0) R;
	R.x /= r.x;
	R.y /= r.y;
	R.w /= r.w;
	R.h /= r.h;
	return R;
}

void nite::Rect::operator=(float c){
	x = c;
	y = c;
	w = c;
	h = c;
}

nite::Rect nite::Rect::operator*(float c){
	Rect R = *this;
	R.x *= c;
	R.y *= c;
	R.w *= c;
	R.h *= c;
	return R;
}

nite::Rect nite::Rect::operator*(const nite::Rect &r){
	Rect R = *this;
	R.x *= r.x;
	R.y *= r.y;
	R.w *= r.w;
	R.h *= r.h;
	return R;
}


nite::Rect nite::Rect::operator+(float c){
	Rect R = *this;
	R.x += c;
	R.y += c;
	R.w += c;
	R.h += c;
	return R;
}

nite::Rect nite::Rect::operator+(const nite::Rect &r){
	Rect R = *this;
	R.x += r.x;
	R.y += r.y;
	R.w += r.w;
	R.h += r.h;
	return R;
}

nite::Rect nite::Rect::operator-(float c){
	Rect R = *this;
	R.x -= c;
	R.y -= c;
	R.w -= c;
	R.h -= c;
	return R;
}

nite::Rect nite::Rect::operator-(const nite::Rect &r){
	Rect R = *this;
	R.x -= r.x;
	R.y -= r.y;
	R.w -= r.w;
	R.h -= r.h;
	return R;
}

bool nite::Rect::operator==(float c){
	return x == c && y == c && w == c && h == c;
}

bool nite::Rect::operator==(const nite::Rect &r){
	return x == r.x && y == r.y && w == r.w && h == r.h;
}

nite::Rect::operator String(){
	return str();
}

String nite::Rect::str(){
	return "("+toStr(x)+", "+toStr(y)+", "+toStr(w)+", "+toStr(h)+")";
}

nite::Vec2 nite::Rect::getSumSides(){
	return nite::Vec2(x + w, y + h);
}

/*
================
Polygon
================
*/
nite::Polygon::Polygon(){
	origin.x = 0;
	origin.y = 0;
}

void nite::Polygon::clear(){
	vert.clear();
	return;
}

void nite::Polygon::add(Vec2 &Point){
	vert.push_back(Point);
	return;
}

void nite::Polygon::add(float x, float y){
	vert.push_back(Vec2(x,y));
	return;
}

void nite::Polygon::setOrigin(float x, float y){
	origin.x = x;
	origin.y = y;
	return;
}

void nite::Polygon::setOrigin(Vec2 &Point){
	origin = Point;
	return;
}
