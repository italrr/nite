#ifndef NITE_NSCRIPT_HPP
	#define NITE_NSCRIPT_HPP

	#include "Types.hpp"

	namespace nite {


        /*
            nite Script File

            nite Script is a very, very basic scripting language for
            nite engine.

            CURRENT STATE:
                nite Script files are parsed line per line, each line
                executed using nite::Console's interpret.

                nite Script files are different from the .jsons
                nite uses them to load configs and other types of data.

            TODO:
                Proposition to make nite Script featureful/complex enough to
                be able to manipulate entire game states.
            
        */

        struct nScriptLine {
            String line;
            size_t n;
            nScriptLine(){

            }
            nScriptLine(const String &line, size_t n){
                this->line = line;
                this->n = n;
            }
        };

        class nScript {
            public:
                nScript();
                nScript(const String &path);
                bool execute(const String &path);
                bool execute();
            private: 
                void init();
                bool load(const String &path);
                Vector<nite::nScriptLine> lines;
                String path; // path + filename
                String hash; // file's hash
                String name;
                UInt64 lastReadTime;
                bool loaded;
        };

    }

#endif