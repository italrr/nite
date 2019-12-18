#ifndef NITE_VIEW_HPP
	#define NITE_VIEW_HPP

	#include "Types.hpp"

	namespace nite {
		float getViewX(unsigned target);
		float getViewY(unsigned target);
		nite::Vec2 getView(unsigned target);
		void setViewPosition(float x, float y, unsigned target);
		void setViewPosition(const nite::Vec2 &P, unsigned target);
		void setView(bool m, unsigned target);
		void setViewPosition(float x, float y);
		void setViewPosition(const nite::Vec2 &P);
		void viewUpdate();
		void shakeScreen(int target, float magnitude, UInt64 timeout);
	};

#endif
