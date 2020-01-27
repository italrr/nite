#ifndef NITE_TOOLS_HPP
	#define NITE_TOOLS_HPP

	#include "../Types.hpp"

	#include "../JSON/Jzon.hpp"
	#include "./hashids.hpp"

	#define NITE_HASH_METHOD "MD5"

	/*
	================
	Log
	================
	*/
	namespace nite {
		void print(const String &Message);
		void print(int n);
		void print(unsigned n);
		void print(float n);
		void print(double n);
		void print(bool n);
		void print(const nite::Vec2 &v);
		void print(char *c);
		void print(const char *c);
		void print(UInt64 n);
		void print(Int64 n);
		//void print(size_t n);
	}

	/*
	================
	Hash
	================
	*/
	namespace nite {
		String hashMemory(char *Data, size_t size);
		String hashFile(const String &path);
		String hashString(const String &str);
	}

	/*
	================
	time
	================
	*/
	namespace nite {
		namespace TM {
			const unsigned D 		= 0;
			const unsigned M 		= 1;
			const unsigned Y 		= 2;
			const unsigned H 		= 3;
			const unsigned MI 		= 4;
			const unsigned S 		= 5;
			const unsigned All 		= 6;
			const unsigned A 		= 6;
		}
		String secondsToText(long long unsigned secs);
		String getTime(unsigned time);
		String getTimestamp();
		void sleep(int time);
		UInt64 getTicks();
		UInt32 getTocks();
	}

	/*
	================
	System
	================
	*/
	namespace nite {
		void exit();
		String getCWD();
	}

	/*
	================
	String
	================
	*/
	namespace nite {
		bool isNumber(String number);
		bool isUpper(const String &Input);
		bool isLower(const String &Input);

		long long int toInt (const String &string);
		double toFloat(const String &number);
		Vector<String> split(const String &text, char sep);
		String toStr(size_t number);
		String toStr(long long int number);
		String toStr(long long unsigned int number);
		String toStr(int number);
		String toStr(unsigned number);
		String toStr(double Float);

		String floatToStr(float number);

		String toUpper(const String &Input);
		String toLower(const String &Input);

		String strLimitChars(const String &Input, unsigned maxChars);

		String strAt(const String &Input,unsigned x);
		String subStr(const String &string,unsigned start,unsigned end);
		void strSplit(String input, const String &split, Vector<String> &list);
		String strRemove(String Input, const String &spec);
		String strRemoveMarks(String Input, const String &Exception);
		String strRemoveCoin(String Input, const String &coinc);
		String strRemoveEndline(String Input);
		String strRemoveSpaces(String Input);
	}

	/*
	================
	Math
	================
	*/
	namespace nite {
		const static float PI = 3.14159265358979323846;
		float toRadians(float an);
		float toDegrees(float an);
		float sin(float an);
		float cos(float an);
		float tan(float an);
		float arcsin(float an);
		float arcos(float an);
		float arctan(float y, float x);
		int randomInt(int min,int max);
		float fractional (float number);
		int round (float number);
		nite::Vec2 round(const nite::Vec2 &v);
		int floor (float number);
		nite::Vec2 floor(const nite::Vec2 &v);
		int ceil (float number);
		nite::Vec2 ceil(const nite::Vec2 &v);
		nite::Vec2 snapPosition(const nite::Vec2 &blockSize, const nite::Vec2 &position);
		long long int factorial(int n);
		nite::Vec2 abs(const nite::Vec2 &v);
		double neg(double n);
		double abs(double n);
		double inv(double n);
		float getAngle(float x, float y);
		double pow(double b, double ex);
		double sqrt(double n);
		int odd(int n);
		float getSign(float n);
		UInt64 timescaled(UInt64 t);
		float distance(const Vec2 &P, const Vec2 &Q);
		float distance(float x1, float y1, float x2, float y2);
		float fractional (float number);
		bool lerp(float &x1, float x2, float Step);
		bool lerpDiscrete(float &x1, float x2, float Step);
		bool lerpAbsolute(float &y1, float y2, float Step);
		float distancePointToLine(const nite::Vec2 &Point, const nite::Vec2 &P1, const nite::Vec2 &P2);
		bool isPointInRegion(const nite::Vec2 &P, const nite::Vec2 &RP, const nite::Vec2 &RQ);
	}
	/*
	================
	Files
	================
	*/
	namespace nite {
		namespace Find {
			const unsigned File 		= 0;
			const unsigned Folder 		= 1;
			const unsigned Directory 	= 1;
			const unsigned Both	 		= 2;
			const unsigned Any	 		= 2;
		}
		bool directoryExists(const String &path);
		bool fileExists(const String &path);
		String getFilename(const String &path);
		String getFilePath(const String &path);
		String getFormat(const String &filename);
		String formatRemove(String filename);
		bool fileFind(const String &path, unsigned Type, String Format, bool R, bool FP, Vector<String> &List);
		size_t fileSize(const String &path);
	}
	/*
	================
	StepTimer
	================
	*/	
	namespace nite {
		struct StepTimer {
			StepTimer(String name);
			StepTimer();
			UInt64 lastTick;
			UInt64 time;
			String name;
			String report;
			void init();
			void end();
			String getStatus();
		};		
	}
	/*
	================
	Async
	================
	*/	
	namespace nite {

		// class AsyncTask;
		// typedef Lambda<void(nite::AsyncTask &context)> AsyncLambda;    
		
		// class AsyncTask {
		// 	private:
		// 		int id;
		// 		nite::AsyncLambda lambda;
		// 		int status;
		// 		UInt64 startTime;
		// 		UInt64 delayTime;
		// 	public:
		// 		AsyncTask();							
		// 		int getStatus();
		// 		void pause();
		// 		void resume();
		// 		void start();
		// 		void stop();
		// 		void step();
		// 		Shared<nite::AsyncTask> spawn(nite::AsyncLambda lambda);
		// 		Shared<nite::AsyncTask> spawn(nite::AsyncLambda lambda, UInt64 delayTime);
		// };

		namespace AsyncTask {

			namespace State {
				const static unsigned Idle = 0;
				const static unsigned Running = 1;
				const static unsigned Paused = 2;
				const static unsigned Killed = 3;
			}
					
			class Context;
			typedef Lambda<void(nite::AsyncTask::Context &context)> ALambda;  

			struct Context {
					int id;
					nite::AsyncTask::ALambda lambda;
					int state;
					UInt64 startTime;
					UInt64 delayTime;
					Context();
					void start();
					void step();
					void stop();
					void pause();
					void resume();
			};

			Shared<nite::AsyncTask::Context> spawn(nite::AsyncTask::ALambda lambda);
			Shared<nite::AsyncTask::Context> spawn(nite::AsyncTask::ALambda lambda, UInt64 delayTime);
					
			void update();
			void end();
		}
		
		
	}
#endif
