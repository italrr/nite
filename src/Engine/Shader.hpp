#ifndef NITE_SHADER_HPP
	#define NITE_SHADER_HPP

	#include "Types.hpp"
	#include "Graphics.hpp"
	namespace nite {
		struct Shader {
				Shader();
				~Shader();
				Shader(const String &frag, const String &vert);
				void load(const String &frag, const String &vert);
				void unload();
				bool isLoaded();
				void reload();
				void applyRenderTarget(unsigned T);
				void applyRenderTarget(unsigned T, Uniform uniforms);
				Shader& operator= (const Shader &other);
				int getProgram();
				bool faulty;
				int id;
				String fragFilename;
				String vertFilename;

		};
	}
#endif
