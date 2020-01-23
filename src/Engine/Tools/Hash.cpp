#include "Tools.hpp"
#include <fstream>

/* see MD5.cpp */
void CalculateMD5(char *buffer, int length, char *checksum);

/*
================
Calculate MD5 from memory
================
*/
String nite::hashMemory(char *Data, size_t size){
	char Buffer[33];
	CalculateMD5(Data, size, Buffer);
	return Buffer;
}

/*
================
Calculate MD5 from string
================
*/
String nite::hashString(const String &str){
	char Buffer[33];
	CalculateMD5((char*)str.c_str(), str.size(), Buffer);
	return Buffer;
}

/*
================
Calculate MD5 from File
================
*/
String nite::hashFile(const String &path){
	char Buffer[33];
	char *Data;
	std::ifstream File;
	File.open(path.c_str(), std::ios::binary | std::ios::ate);
	if (!File.is_open()){
		return "#ERROR";
	}
	size_t size = File.tellg();
	Data = new char[size];
	File.seekg(0, std::ios::beg);
	File.read(Data, size);
	File.close();
	CalculateMD5(Data, size, Buffer);
	delete[] Data;
	return Buffer;
}
