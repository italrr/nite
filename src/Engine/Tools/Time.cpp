#include <unistd.h>
#include <sys/time.h>

#ifdef _WIN32
	#include <windows.h>
#endif
#include <sys/time.h>

#include "Tools.hpp"

/*
================
Convert seconds into text
3690 seconds would be 1 hour, 1 minutes and 30 seconds
================
*/
String nite::secondsToText(long long unsigned secs){
	unsigned hours = secs/3600.0;
	float fractpartminutes = nite::fractional(secs/3600.0);
	unsigned minutes = fractpartminutes*60.0;
	float fractpartseconds = nite::fractional(fractpartminutes*60.0);
	unsigned seconds = nite::round(fractpartseconds*60.0);
	String output = "";
	if (hours){
		output += nite::toStr(hours)+" hour";
		if (hours>1){
			output += "s";
		}
		if (minutes>=0)
			output += ", ";
	}
	if (minutes or (hours and seconds)){
		output += nite::toStr(minutes)+" minute";
		if (minutes>1)
			output += "s";
		if (seconds)
			output += " and ";
	}
	if (seconds or (!hours and !minutes)){
		output += nite::toStr(seconds)+" second";
		if (seconds>1){
			output += "s";
		}
	}
	return output;
}

/*
================
Get the current timestamp:
12:12:12
================
*/
String nite::getTimestamp(){
	String Timestamp = nite::getTime(nite::TM::H)+":";
	Timestamp += nite::getTime(nite::TM::MI)+":";
	Timestamp += nite::getTime(nite::TM::S);
	return Timestamp;
}

/*
================
Get time depending on the input type time specified in nite/Tools.h
getTime(nite::TM::D) would return the current week day
================
*/
String nite::getTime(unsigned timeType){
    time_t currentTime;
    time (&currentTime);
    tm * ptm= localtime(&currentTime);

	switch (timeType){
		case nite::TM::D:
			return toStr(ptm->tm_mday);
		break;
		case nite::TM::M:
			return toStr(ptm->tm_mon);
		case nite::TM::Y:
			return toStr(ptm->tm_year);
		break;
		case nite::TM::H:
			return toStr(ptm->tm_hour);
		break;
		case nite::TM::MI:
			return toStr(ptm->tm_min);
		break;
		case nite::TM::S:
			return toStr(ptm->tm_sec);
		break;
		case nite::TM::All:
			return nite::strRemoveEndline(ctime(&currentTime));
		break;
	}

	return "NULL";
}

/*
================
Returns seconds since epoch
================
*/
UInt32 nite::getTocks(){
    static timeval t;
    gettimeofday(&t, 0);
    return t.tv_sec;
}

/*
================
Returns milliseconds since an arbitrary time
================
*/
UInt64 nite::getTicks(){
    static timeval t;

    gettimeofday(&t, 0);

    //time_t currentTime;
    //time (&currentTime);
    //tm * ptm= localtime(&currentTime);

   // unsigned d = ptm->tm_sec*1000.0 + ptm->tm_min*60.0*1000.0 + ptm->tm_hour*3600.0*1000.0;

    return t.tv_sec*1000.0 + (t.tv_usec/1000.0);
}

/*
================
Sleep the main thread in milliseconds
================
*/
#ifdef _WIN32
	static inline void __usleep(__int64 usec){
	    HANDLE timer;
	    LARGE_INTEGER ft;

	    ft.QuadPart = -(10*(usec*1000));
	    timer = CreateWaitableTimer(NULL, TRUE, NULL);
	    SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
	    WaitForSingleObject(timer, INFINITE);
	    CloseHandle(timer);
	}
#else
	#include <chrono>
	#include <thread>
	static inline void __usleep(int msec){
		std::this_thread::sleep_for(std::chrono::milliseconds(msec));
	}
#endif

void nite::sleep(int _time){
    __usleep(_time);
}
