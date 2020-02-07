#ifndef NITE_CONSOLE_HPP
    #define NITE_CONSOLE_HPP

    #include "Tools/Tools.hpp"

    namespace nite {
        namespace Console {
            namespace ProxyType {
                enum ProxyType : UInt8 {
                    Int = 0, // System base integer
                    Float,
                    Bool
                };
            }

            typedef void (*Function)(Vector<String> parameters);
            // typedef std::function<void(Vector<String>) Lambda;
            bool createProxy(const String &name, int type, size_t s, void *ref);
            bool createProxy(const String &name, int type, size_t s, void *ref, nite::Console::Function function);
            bool createFunction(const String &name, nite::Console::Function function);
            // bool createFunction(const String &name, nite::Console::Lambda &lambda);
            void *getProxyReference(const String &name);
            void add(const String &input, bool print = true);
            void add(const String &input, const nite::Color &color, bool print = true);
            void interpret(const String &command, bool remoteExec);
            void open();
            void close();
            bool isOpen();
            void render();
            void update();
            void end();
            struct CreateProxy {
                CreateProxy(const String &name, int type, size_t s, void *ref);
                CreateProxy(const String &name, int type, size_t s, void *ref, nite::Console::Function function);
            };
            struct CreateFunction {
                CreateFunction(const String &name, nite::Console::Function function);
                // CreateFunction(const String &name, nite::Console::Lambda &lambda);
            };
        }
    }

#endif
