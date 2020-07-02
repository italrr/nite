#include "Tools.hpp"
#include <time.h>
#include <math.h>
#include <cmath>
#include <random>

# define M_PI 3.14159265358979323846  /* pi -> lol you dont say? */

float nite::toRadians(float an){
	return (-an * M_PI) / 180.0f; // inverted y
}

float nite::getSign(float n){
	return n<0 ? -1 : 1;
}

float nite::toDegrees(float an){
	auto rads = -an * (180.0f / M_PI); // inverted y again
	return rads < 0 ? rads * -1.0f : rads; // from 0 to 360 
}

float nite::sin(float an){
	return std::sin(an);
}

int nite::odd(int n){
	return n % 2 == 0 ? n + 1 : n;
}

float nite::cos(float an){
	return std::cos(an);
}

float nite::tan(float an){
	return std::tan(an);
}

float nite::arcsin(float an){
	return std::asin(an);
}

float nite::arcos(float an){
	return std::acos(an);
}

float nite::arctan(float y, float x){
	return std::atan2(y, x);
}

bool nite::isPointInRegion(const nite::Vec2 &P, const nite::Vec2 &RP, const nite::Vec2 &RQ){
	return P.x>=RP.x && P.x<=RQ.x && P.y>=RP.y && P.y<=RQ.y;
}

float nite::distancePointToLine(const nite::Vec2 &Point, const nite::Vec2 &P1, const nite::Vec2 &P2){
	float A = Point.x - P1.x;
	float B = Point.y - P1.y;
	float C = P2.x - P1.x;
	float D = P2.y - P1.y;
	float Dot = A*C+B*D;
	float lenSq = nite::pow(C, 2) + nite::pow(D, 2);
	float Param = Dot/lenSq;
	float xx, yy;
	if (Param < 0 || (P1.x == P2.x && P1.y == P2.y)){
		xx = P1.x;
		yy = P1.y;
	}else
	if (Param > 1){
		xx = P2.x;
		yy = P2.y;
	}else{
		xx = P1.x + Param * C;
		yy = P1.y + Param * D;
	}
	float dx = Point.x - xx;
	float dy = Point.y - yy;
	return nite::sqrt(nite::pow(dx, 2) + nite::pow(dy, 2));
}

nite::Vec2 nite::snapPosition(const nite::Vec2 &blockSize, const nite::Vec2 &position){
	nite::Vec2 P = position, Q = blockSize;
	return nite::floor(P/Q)*Q;
}

float nite::distance(const Vec2 &P, const Vec2 &Q){
	return std::sqrt( std::pow(Q.x-P.x,2) + nite::pow(Q.y-P.y,2) );
}

float nite::distance(float x1, float y1, float x2, float y2){
	return std::sqrt( std::pow(x2-x1,2) + nite::pow(y2-y1,2) );
}

#include "../Graphics.hpp"
bool nite::cInterp(float &x1, float x2, float _Step){ // Apply game's delta
	if(x1 == x2) return true;
	if(_Step > 1.0f){
		_Step = 1.0f;
	}
	float Step = 1.0f - nite::pow(1.0f - _Step, nite::getDelta() * nite::getTimescale() * 0.077f);
	x1 = (1.0f-Step)*x1 + Step*x2;
	if(abs(x1-x2)<0.10f) x1 = x2; // TODO: Check this!
	return false;
}

bool nite::cInterpDiscrete(float &x1, float x2, float _Step){
	if(x1 == x2) return true;
	if(_Step > 1.0f){
		_Step = 1.0f;
	}	
	float Step = 1.0f - nite::pow(1.0f - _Step, nite::getDelta() * 0.077f);
	x1 = (1.0f-Step)*x1 + Step*x2;
	// if(abs(x1-x2)<1.0f) x1 = x2;
	if(abs(x1-x2)<0.10f) x1 = x2; // TODO: Check this!	
	return false;
}

bool nite::cInterpAbsolute(float &x1, float x2, float Step){
	if(x1 == x2) return true;
	if(Step > 1.0f){
		Step = 1.0f;
	}	
	x1 = (1.0f-Step)*x1 + Step*x2;
	if(abs(x1-x2)<1.0f) x1 = x2;
	return false;
}

bool nite::lerpDiscrete(float &x1, float x2, float step){
	if(x1 == x2) return true;
	if(step > 1.0f){
		step = 1.0f;
	}	
	float f = (1.0f - nite::pow(1.0f - step, nite::getDelta() * 0.077f));
	// x1 = x1 + f * (x2 - x1);	
	x1 = (x1 * (1.0 - f)) + (x2 * f);
	return false;
}

UInt64 nite::timescaled(UInt64 t){
	return (1.0f / nite::getTimescale()) * t;
}

long long int factorial(int n){
    long long int h=1;
    if (n<0)
        n *= -1;
    for (int i=1; i<=n; ++i){
        h=h*i;
	}
    if (n<0)
        h *= -1;
    return h;
}

float nite::fractional (float number){
    return (float)number-((int)number);
}

int nite::ceil (float number){
    return std::ceil(number);
}

nite::Vec2 nite::ceil(const nite::Vec2 &v){
	return nite::Vec2(nite::ceil(v.x), nite::ceil(v.y));
}

int nite::floor (float number){
	return std::floor(number);
}

nite::Vec2 nite::floor(const nite::Vec2 &v){
	return nite::Vec2(nite::floor(v.x), nite::floor(v.y));
}

int nite::round(float number){
    return std::round(number);
}

nite::Vec2 nite::round(const nite::Vec2 &v){
	return nite::Vec2(nite::round(v.x), nite::round(v.y));
}

double nite::abs(double n){
	return (n<0 ? n*-1 : n);
}

nite::Vec2 nite::abs(const nite::Vec2 &v){
	return nite::Vec2(nite::abs(v.x), nite::abs(v.y));
}

double nite::neg(double n){
	return (n>0 ? n*-1 : n);
}

double nite::inv(double n){
	return n*-1.0;
}

double nite::pow(double b, double ex){
	return std::pow(b,ex);
}

double nite::sqrt(double n){
	return std::sqrt(n);
}

float nite::getAngle(float x, float y){
	float angle = atan2(y,x)/M_PI*180.0;
	if (angle<=0){
		angle *= -1.0;
	}else{
		angle +=180.0;
	}
	return angle;
}

int nite::randomInt(int min, int max){
	static std::random_device rd;
	std::mt19937 rng(rd());
	std::uniform_int_distribution<int> uni(min,max);
	return uni(rng);
}
