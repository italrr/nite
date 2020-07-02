#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Tools.hpp"
#include "MD5.hpp"

/*
================
Calculate MD5 from memory
================
*/
String nite::hashMemory(char *Data, size_t size){
	return md5(Data, size);
}

/*
================
Calculate MD5 from string
================
*/
String nite::hashString(const String &str){
	return md5(str);
}

/*
================
Calculate MD5 from File
================
*/
String nite::hashFile(const String &path){
	if(!nite::fileExists(path)){
		nite::print("failed to hash file '"+path+"': it doesn't exist");
		return "";
	}
	FILE *f = fopen(path.c_str(), "rb");
	fseek(f, 0, SEEK_END);
	size_t fsize = ftell(f);
	fseek(f, 0, SEEK_SET);
	char *buffer = (char*)malloc(fsize);
	fread(buffer, fsize, 1, f);
	fclose(f);
	String hash = md5(buffer, fsize);
	free(buffer);
	return hash;
}
