#ifndef NITE_TYPES_HPP
	#define NITE_TYPES_HPP

	#include <vector>
	#include <unordered_map>
	#include <string>
	#include <memory>
	#include <functional>

	#define Shared	std::shared_ptr
	#define Lambda	std::function
	#define String	std::string
	#define Vector 	std::vector
	#define Dict	std::unordered_map

	#define Int8 	int8_t
	#define UInt8 	uint8_t

	#define Int16 	int16_t
	#define UInt16 	uint16_t

	#define Int32 	int32_t
	#define UInt32 	uint32_t

	#define Int64 	int64_t
	#define UInt64 	uint64_t


	/*
	================
	nite Specific Types
	================
	*/
	namespace nite {
		/* Type color */
		struct Color {
			void rectify();
			void operator=(float c);
			bool operator<(const nite::Color &v);
			bool operator>(const nite::Color &v);
			bool operator<=(const nite::Color &v);
			bool operator>=(const nite::Color &v);
			bool operator<(float c);
			bool operator>(float c);
			bool operator<=(float c);
			bool operator>=(float c);
			bool operator==(float c);
			bool operator==(const nite::Color &v);
			bool operator!=(float c);
			bool operator!=(const Color &v);
			nite::Color operator/(float c);
			nite::Color operator/(const nite::Color &v);
			nite::Color operator*(float c);
			nite::Color operator*(const nite::Color &v);
			nite::Color operator+(float c);
			nite::Color operator+(const nite::Color &v);
			nite::Color operator-(float c);
			nite::Color operator-(const nite::Color &v);
			nite::Color invert();
			Color(const String &hex);
			Color();
			Color(const nite::Color &color);
			Color(float r, float g, float b);
			Color(float r, float g, float b, float a);
			Color(float c);
			Color(float c, float a);
			void set(float c, float a);			
			void set(float c);
			void set(const String &hex);
			void set(float r, float g, float b, float a);
			void set(const nite::Color &color);
			float r;
			float g;
			float b;
			float a;
			void to100();
			void to1();			
			operator String();
			String str();
			String hex();
			bool cInterp(const Color &v, float Step);
			bool cInterpDiscrete(const Color &v, float Step);
		};
		const nite::Color White(1.0f, 1.0f ,1.0f);
		const nite::Color Black(0, 0 ,0);
		const nite::Color Red(1.0f, 0, 0);
		const nite::Color DKRed(0.4f, 0, 0);
		const nite::Color Blue(0,0,1.0f);
		const nite::Color Green(0,1.0f,0);
		const nite::Color Yellow(1.0f,1.0f,0);
		const nite::Color LTYellow(1.0f,0.95f,0.30f);
		const nite::Color Pink(0.95f,0.10f,0.65f);
		const nite::Color Orange(1.0f,0.3f,0);
		const nite::Color Purple(0.5f,0,1.0f);
		const nite::Color Gray(0.10f,0.10f,0.10f);
		const nite::Color DKGray(0.3f,0.3f,0.3f);
		/* Type Vector 2D */
		struct Vec2 {
			Vec2();
			Vec2(float X, float Y);
			Vec2(const Vec2 &Vec);
			Vec2(float c);
			float x;
			float y;
			float avg();
			void operator=(float c);
			bool operator<(const nite::Vec2 &v);
			bool operator>(const nite::Vec2 &v);
			bool operator<=(const nite::Vec2 &v);
			bool operator>=(const nite::Vec2 &v);
			bool operator<(float c);
			bool operator>(float c);
			bool operator<=(float c);
			bool operator>=(float c);
			bool operator==(float c);
			bool operator==(const Vec2 &v);
			bool operator!=(float c);
			bool operator!=(const Vec2 &v);
			Vec2 operator/(float c);
			Vec2 operator/(const Vec2 &v);
			Vec2 operator*(float c);
			Vec2 operator*(const Vec2 &v);
			Vec2 operator+(float c);
			Vec2 operator+(const Vec2 &v);
			Vec2 operator-(float c);
			Vec2 operator-(const Vec2 &v);
			void set(const Vec2 &v);
			void set(float X, float Y);
			float dotProduct(const nite::Vec2 &v);
			float crossProduct(const nite::Vec2 &v);
			nite::Vec2 crossProduct(float s);
			// cInterp uses game ticks and engine ticks
			bool cInterp(const Vec2 &v, float step);
			// cInterpDiscrete uses only engine ticks, so if the game slows down, cInterpDiscrete is unaffected
			bool cInterpDiscrete(const Vec2 &v, float step);
			// cInterpAbsolute doesn't use any ticks for interpolation, meaning any slowdowns will directly affect the transitions
			bool cInterpAbsolute(const Vec2 &v, float step);
			bool lerp(const Vec2 &v, float step, float limit = 0.0f);
			bool lerpDiscrete(const Vec2 &v, float step, float limit = 0.0f);
			String str();
			operator String();
		};

		/* Type Rectangle */
		struct Rect {
			Rect();
			Rect(float x, float y, float w, float h);
			Rect(float x);			
			Rect(const Rect &r);
			float x;
			float y;
			float w;
			float h;
			void set(const Rect &r);
			void set(float c);
			void set(float X, float Y, float W, float H);
			void set(const Vec2 &P, const Vec2 &Q);
			void operator=(float c);
			bool operator<(const Rect &r);
			bool operator>(const Rect &r);
			bool operator<=(const Rect &r);
			bool operator>=(const Rect &r);
			bool operator<(float c);
			bool operator>(float c);
			bool operator<=(float c);
			bool operator>=(float c);
			bool operator==(float c);
			bool operator==(const Rect &r);
			bool operator!=(float c);
			bool operator!=(const Rect &r);
			Rect operator/(float c);
			Rect operator/(const Rect &r);
			Rect operator*(float c);
			Rect operator*(const Rect &r);
			Rect operator+(float c);
			Rect operator+(const Rect &r);
			Rect operator-(float c);
			Rect operator-(const Rect &r);
			nite::Vec2 getSumSides();
			String str();
			operator String();
		};

		/* Type Polygon */
		struct Polygon {
			Polygon();
			Polygon(int n, const nite::Vec2 &v);
			void rectangle(float w, float h);
			void rectangle(const nite::Vec2 &size);
			nite::Polygon rotate(float rads);
			void clear();
			void add(nite::Vec2 &point);
			void add(float x, float y);
			void setOrigin(float x, float y);
			void setOrigin(Vec2 &Point);
			
			Polygon operator+(float c);
			Polygon operator+(const Vec2 &v);			

			Polygon operator-(float c);
			Polygon operator-(const Vec2 &v);	

			Polygon operator*(float c);
			Polygon operator*(const Vec2 &v);	

			Polygon operator/(float c);
			Polygon operator/(const Vec2 &v);							

			Vector <nite::Vec2> vert;
			nite::Vec2 origin;
		};
	}

#endif
