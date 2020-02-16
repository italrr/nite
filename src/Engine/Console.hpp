#ifndef NITE_CONSOLE_HPP
    #define NITE_CONSOLE_HPP

    #include "Tools/Tools.hpp"

    namespace nite {
        namespace Console {

            namespace ProxyType {
                enum ProxyType : UInt8 {
                    Int = 0,
                    Float,
                    Bool
                };
            }

            struct Result {
                String msg;
                nite::Color color;
                Result(){
                    msg = "";
                    color = nite::Color(1.0f, 1.0f, 1.0f, 1.0f);
                }
                Result(const String &msg, const nite::Color &color){
                    this->msg = msg;
                    this->color.set(color);
                }                
            };

            typedef nite::Console::Result (*Function)(Vector<String> parameters);
            typedef void (*PipeFunction)(const String &input);
            void pipeServerSideExecs(nite::Console::PipeFunction function);
            bool createProxy(const String &name, int type, size_t s, void *ref, nite::Console::Function function, bool serverSide, bool adminType);
            bool createFunction(const String &name, nite::Console::Function function, bool serverSide, bool adminType);
            void *getProxyReference(const String &name);
            void add(const String &input, bool print = true);
            void add(const String &input, const nite::Color &color, bool print = true);
            void add(const nite::Console::Result &result, bool print = true);
            nite::Console::Result interpret(const String &command, bool remoteExec, bool svExec = false, bool asAdmin = false);
            void open();
            void close();
            bool isOpen();
            void render();
            void update();
            void end();
            struct CreateProxy {
                CreateProxy(const String &name, int type, size_t s, void *ref);
                CreateProxy(const String &name, int type, size_t s, void *ref, bool serverSide , bool adminType = false);
                CreateProxy(const String &name, int type, size_t s, void *ref, nite::Console::Function function);
                CreateProxy(const String &name, int type, size_t s, void *ref, nite::Console::Function function, bool serverSide, bool adminType = false);
            };
            struct CreateFunction {
                CreateFunction(const String &name, nite::Console::Function function);
                CreateFunction(const String &name, nite::Console::Function function, bool serverSide, bool adminType = false);
            };
        }
    }

#endif
