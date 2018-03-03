#ifndef NITE_AUDIO_H
  #define NITE_AUDIO_H

  #include "Types.hpp"

  namespace nite {

    namespace AudioMode {
      static const unsigned Stream = 0;
      static const unsigned Memory = 1;
    }

    class Audio {
      public:
        Audio(const String &filename, unsigned mode);
        Audio(const String &filename);
        Audio(const nite::Audio &sound);
        Audio();
        ~Audio();
        void load(const String &filename, unsigned mode);
        void load(const String &filename);
        void unload();
        int play(bool loop,float vol, float pitch);
        int play(bool loop);
        void resume(int id);
        void stop(int id);
        void pause(int id);
        nite::Audio& operator= (const nite::Audio &other);
      private:
        String filename;
        int mode;
        int objectId;
        bool loaded;
    };

  }

#endif
