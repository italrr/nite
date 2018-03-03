#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include "Tools.hpp"

bool nite::isNumber(String number){
	static const String numbers[]={
		"0","1","2","3","4","5",
		"6","7","8","9","."
	};
	static const unsigned size = sizeof(numbers)/sizeof(String);

	/* remove signs */
	if (nite::strAt(number,0)=="+" or nite::strAt(number,0)=="-")
		number.erase(0,1);

	if (number.length()==0)
		return 0;
	bool n=0;
	for (unsigned i=0; i<number.length(); i++){
		n=0;
		for (unsigned e=0; e<size; e++){
			if (nite::strAt(number,i)==numbers[e])
				n=1;
		}
		if (n==0)
			return 0;
	}
    return 1;

}

/*
================
Check if the string is completely uppercase
================
*/
bool nite::isUpper(const String &Input){
	for (unsigned i=0; i<Input.length(); i++){
		char c = Input.at(i);
		if (!isupper(c))
			return 0;
	}
	return 1;
}

/*
================
Check if the string is completely lowercase
================
*/
bool nite::isLower(const String &Input){
	for (unsigned i=0; i<Input.length(); i++){
		char c = Input.at(i);
		if (!islower(c))
			return 0;
	}
	return 1;
}

/*
================
Turn a string to uppercase
================
*/
String nite::toUpper(const String &Input){
	String Output;
	for(unsigned i=0; i<Input.size(); i++){
		Output += toupper(Input.at(i));
	}
	return Output;
}

/*
================
Convert a string to lowercase
================
*/
String nite::toLower(const String &Input){
	String Output;
	for(unsigned i=0; i<Input.size(); i++){
		Output += tolower(Input.at(i));
	}
	return Output;
}

/*
================
Convert a string to int
================
*/
long long int nite::toInt (const String &string){
    return atoll(string.c_str());
}

/*
================
Convert a string to float
================
*/
double nite::toFloat(const String &s){
	char *Buffer = (char*)s.c_str();
	return atof(Buffer);
}

/*
================
Convert a float to string with 3 digits of precision.
================
*/
String nite::floatToStr(float number){
    char buffer[64];
	sprintf(buffer,"%.3f", number);
    return buffer;
}

/*
================
Convert a double/float to string
================
*/
String nite::toStr(double Float){
    char Buffer[64];
	sprintf(Buffer,"%.4f",Float);
	String s = Buffer;
	for (unsigned i=s.length()-1; i>0; i--){
		if (nite::strAt(s,i)=="."){
			s.erase(i,1);
			break;
		}
		if (nite::strAt(s,i)!="0")
			break;
		if (nite::strAt(s,i)=="0"){
			s.erase(i,1);
		}
	}
	return s;
}


/*
================
If the Input is longer than maxChars, it will shrink it with "..."
================
*/
String nite::strLimitChars(const String &Input, unsigned maxChars){
	String str = Input;
	if (str.length() <= maxChars) return str;
	str = nite::subStr(str, 0, maxChars - 4);
	str += "...";
	return str;
}

/*
================
Convert a long long int (usually 64 bits depending on the system) to string
================
*/
String nite::toStr(long long int number){
    char buffer[64];
	sprintf(buffer,"%lld",number);
	String out = buffer;
    return out;
}

#ifndef _WIN32

String nite::toStr(size_t number){
	char buffer[64];
sprintf(buffer,"%zu",number);
String out = buffer;
	return out;

}

#endif

/*
================
Convert a long long unsigned int (usually 64 bits depending on the system) to string
================
*/
String nite::toStr(long long unsigned int number){
    char buffer[64];
	sprintf(buffer,"%lld",number);
	String out = buffer;
    return out;
}

/*
================
Convert a int to string
================
*/
String nite::toStr(int number){
    char buffer[64];
	sprintf(buffer,"%i",number);
	String out = buffer;
    return out;
}

/*
================
Convert an unsigned int to string
================
*/
String nite::toStr(unsigned number){
    char buffer[64];
	sprintf(buffer,"%u",number);
	String out = buffer;
    return out;
}

/*
================
Get the fragment in the position specified of the string
================
*/
String nite::strAt(const String &Input, unsigned x){
	if (x>Input.length())
		return "";
	String output = Input;
	output = output.at(x);
	return output;
}

/*
================
remove all the "spec" from the string
================
*/
String nite::strRemove(String Input, const String &spec){
	while ((signed)Input.find(spec)!=-1){
		Input.erase(Input.find(spec),spec.length());
	}
	return Input;
}

/*
================
remove all the spaces
================
*/
String nite::strRemoveSpaces(String Input){
	while ((signed)Input.find(" ")!=-1){
		Input.erase(Input.find(" "),1);
	}
	return Input;
}

/*
================
remove all the endlines
================
*/
String nite::strRemoveEndline(String Input){
	String Buffer;
	if (Input.length()>0){
		Buffer = nite::strAt(Input,Input.length()-1);
		while ( (Buffer=="\n" or Buffer=="\r" )  && Input.length()>0 ){
			Input.erase(Input.length()-1,1);
			if (Input.length()==0)
				break;
			Buffer = Input.at(Input.length()-1);
		}
	}
	return Input;
}

/*
================
remove all the coincidences
================
*/
String nite::strRemoveCoin(String Input, const String &c){
	while (1){
		bool Done = 1;
		for(unsigned i=0; i<Input.length(); i++){
			if (Input.find(c, i) == i and Input.find(c, i+1) == i+c.length()){
				Done = 0;
				Input.erase(i, c.length());
			}
		}
		if (Done){
			break;
		}
	}
	return Input;
}

/*
================
remove special ASCII marks
================
*/
String nite::strRemoveMarks(String Input, const String &Exception){
    static const String Marks[] = {
        "!","?",",","'",".",";",":","(",")","[","]",
        "*","@","#","$","%","^","&","-","_","+","=",
    };
    static const unsigned n = (sizeof(Marks)/sizeof(String));
    for (unsigned i=0; i<n; i++){
		if ((signed)Input.find(Marks[i],0)!=-1 && (signed)Exception.find(Marks[i],0)==-1){
				Input.erase(Input.find(Marks[i],0),1);
		}
    }
    return Input;
}

/*
================
Get a substring unsigned a start and end
It'll return "" (empty) if the parameters are invalid
================
*/
String nite::subStr(const String &string, unsigned start, unsigned end){
    if (start<string.length() and start>=0){
        String sub = "";
        for (unsigned i=start; i<=end; i++){
            sub+=string.at(i);
        }
        return sub;
    }
    return "";
}

/*
================
Split a string using "split" as split reference
================
*/
void nite::strSplit(String input, const String &split, Vector<String> &list){
	if ( !list.size() ){
		list.clear();
	}

	input = nite::strRemoveCoin(input, split);

	while (1){
		int k = input.find(split);

		if (k == -1){
			if (input.length()>0){
				list.push_back(input);
			}
			break;
		}

		if (k == (int)input.length()-1){
			input.erase(input.length()-1, split.length());
			continue;
		}

		if (k == 0){
			input.erase(0, split.length());
			continue;
		}

		list.push_back(subStr(input, 0,k-1));
		input = nite::subStr(input, k+split.length(), input.length()-1);
	}

	return;
}

Vector<String> nite::split(const String &text, char sep) {
  std::vector<std::string> tokens;
  std::size_t start = 0, end = 0;
  while ((end = text.find(sep, start)) != std::string::npos) {
    tokens.push_back(text.substr(start, end - start));
    start = end + 1;
  }
  tokens.push_back(text.substr(start));
  return tokens;
}
