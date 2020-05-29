#include "nScript.hpp"
#include "Tools/Tools.hpp"
#include "Console.hpp"
#include <fstream>

nite::nScript::nScript(){
    init();
}

void nite::nScript::init(){
    loaded = false;
    lastReadTime = nite::getTicks();
    name = "VIRGIN";
    hash = "";
    lines.clear();
}

nite::nScript::nScript(const String &path){
    init();
    load(path);
}

bool nite::nScript::load(const String &path){
    this->loaded = false;
    this->lines.clear();
    if(!nite::fileExists(path)){
        nite::Console::add("failed to read nScript file '"+path+"': It doesn't exist.", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
        return false;
    }
    this->hash = nite::hashFile(path);
    this->name = nite::getFilename(path);
    this->path = path;
    this->lastReadTime = nite::getTicks();

    std::ifstream file(path);
    size_t n = 0;
    if (file.is_open()) {
        std::string line;
        while (getline(file, line)) {
            n++;
            this->lines.push_back(nScriptLine(line, n));
        }
        file.close();
    }
    
    this->loaded = true;
    return true;
}

bool nite::nScript::execute(const String &path){
    load(path);
    return execute();
}

bool nite::nScript::execute(){
    if(!loaded){
        nite::Console::add("can't run nScript. there's nothing loaded.", nite::Color(0.80f, 0.15f, 0.22f, 1.0f));
        return false;
    }
    nite::Console::add("about to run '"+path+"'", nite::Color(0.80f, 0.80f, 0.22f, 1.0f));
    UInt64 startTime = nite::getTicks();
    for(auto i = 0; i < lines.size(); ++i){
        nite::Console::interpret(lines[i].line, true);
    }
    nite::Console::add("ran '"+path+"'. Time "+nite::toStr(nite::getTicks()-startTime)+" msecs", nite::Color(0.32f, 0.25f, 0.80f, 1.0f));
    return true;
}
