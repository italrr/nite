#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include "Tools.hpp"

#ifdef _WIN32
	#include <direct.h>
	#define CWD	_getcwd
#else
	#include <unistd.h>
	#define CWD	getcwd
#endif

/*
================
exit
================
*/
void nite::exit(){
    std::exit(0);
    return;
}

/*
================
Get current working directory
================
*/
String nite::getCWD(){
	char CurrentPath[FILENAME_MAX];
	CWD(CurrentPath, sizeof(CurrentPath));
	return CurrentPath;
}
