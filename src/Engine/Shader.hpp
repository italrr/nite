#ifndef NITE_SHADER
	#define NITE_SHADER

	#include "Types.hpp"
	#include "Graphics.hpp"
	namespace nite {
		class Shader {
			public:
				Shader();
				~Shader();
				Shader(const String &frag, const String &vert);
				void load(const String &frag, const String &vert);
				void unload();
				bool isLoaded();
				void reload();
				void applyRenderTarget(unsigned T);
				void applyRenderTarget(unsigned T, Uniform uniforms);
				int getProgram();
				String getName();
				Shader& operator= (const Shader &other);
				String getFragFilename();
				String getVertFilename();
				bool faulty;
			private:
				String fragFilename;
				String vertFilename;
				int objectId;
				String name;

		};
	}
#endif
