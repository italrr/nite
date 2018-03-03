#ifndef NITE_CONSOLE_H
  #define NITE_CONSOLE_H

  #include "Tools/Tools.hpp"

  namespace nite {
    namespace Console {
      namespace ProxyType {
        const static unsigned Int = 0; // System base integer
        const static unsigned Float = 1;
        const static unsigned Bool = 2;
      }

      typedef void (*Function)(Vector<String> parameters);
      bool createProxy(const String &name, int type, size_t s, void *ref);
      bool createProxy(const String &name, int type, size_t s, void *ref, nite::Console::Function function);
      bool createFunction(const String &name, nite::Console::Function function);
      void *getProxyReference(const String &name);
      void add(const String &input);
      void add(const String &input, const nite::Color &color);
      void interpret(const String &command);
      void open();
      void close();
      bool isOpen();
      void render();
      void update();
      struct CreateProxy {
        CreateProxy(const String &name, int type, size_t s, void *ref);
        CreateProxy(const String &name, int type, size_t s, void *ref, nite::Console::Function function);
      };
      struct CreateFunction {
        CreateFunction(const String &name, nite::Console::Function function);
      };
    }
  }

#endif
