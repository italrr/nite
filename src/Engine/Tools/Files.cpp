#include "Tools.hpp"
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string>


/*
================
Get file's name
================
*/
String nite::getFilename(const String &path){
	int k = path.rfind("/");
	if (k != -1){
		return nite::subStr(path, k+1, path.length()-1);
	}
    return "";
}

/*
================
Get file's path
================
*/
String nite::getFilePath(const String &path){
	int k = path.rfind("/");
	if (k != -1){
		return nite::subStr(path, 0, k-1);
	}
    return "";
}

/*
================
remove .anything from input
================
*/
String nite::formatRemove(String filename){
	int k = filename.rfind(".");
	if (k != -1){
		filename = nite::subStr(filename, 0, k-1);
	}
	return filename;
}

/*
================
Get .anything from input
================
*/
String nite::getFormat(const String &filename){
	int k = filename.rfind(".");
	if(k != -1){
		return nite::subStr(filename, k, filename.length()-1);
	}
	return "";
}

bool nite::removeFile(const String &path){
	if(!nite::fileExists(path)){
		return false;
	}
	remove(path.c_str());
	return true;
}

/*
================
Find files/directories with/out full path and recursively or not
================
*/
bool nite::fileFind(const String &path, unsigned Type, String Format, bool R, bool FP, Vector<String> &List){

    DIR *Directory;
    struct dirent *ent;

    Vector <String> Formats;
    Format = nite::strRemoveSpaces(Format);
    nite::strSplit(Format, "|", Formats);
    bool Anyformat = !((int)Formats.size());

	//Open directory
    Directory = opendir( path.c_str() );

    //Check if the dir if not null
    if (Directory == NULL){
		return false;
	}

	String filename;

	while ( (ent = readdir (Directory)) ) {
		filename = ent->d_name;

		/* Skip '.' and '..' */
		if ( filename == "." or filename == ".."  ){
			continue;
		}

        String FullPath = path + filename;

		struct stat Filetype;
		stat(FullPath.c_str(), &Filetype);

		/* Directory */
		if (S_ISDIR(Filetype.st_mode)) {
			if ((Type == nite::Find::Folder or Type == nite::Find::Any) and Anyformat){
				List.push_back(FP ? FullPath : filename);
			}
			if (R){
				nite::fileFind(FullPath+"/", Type, Format, R, FP, List);
			}
		}else
		/* File */
		if (S_ISREG(Filetype.st_mode)){
			if (Type == nite::Find::File or Type == nite::Find::Any){
				bool a = Anyformat;
				if (!Anyformat){
					for (int i=0; i<Formats.size(); i++){
						if (nite::getFormat(FullPath) == Formats[i]){
							a = 1;
							break;
						}
					}
				}
				if (a){
					List.push_back(FP ? FullPath : filename);
				}
			}
		}

	}
    closedir (Directory);
    return true;
}

/*
================
Check if a file exists
================
*/
bool nite::fileExists(const String &path){
    struct stat tt;
    stat(path.c_str(), &tt);
    return S_ISREG(tt.st_mode);
}

/*
================
Check if a directory exists
================
*/
bool nite::directoryExists(const String &path){
    struct stat tt;
    stat(path.c_str(), &tt);
    return S_ISDIR(tt.st_mode);
}

/*
================
Get file size in bytes
================
*/
size_t nite::fileSize(const String &path){
    if (!nite::fileExists(path)){
        return 0;
    }
    struct stat tt;
    stat(path.c_str(), &tt);
    return tt.st_size;
}
