#ifndef NITE_GRAPHICS_HPP
	#define NITE_GRAPHICS_HPP
	#include "Types.hpp"
	namespace nite {
		const static int DepthTop = -2147483647;
		const static int DepthMiddle = 0;
		const static int DepthBottom = 2147483647;
		const static unsigned RenderTargetPrevious = 0;
		const static unsigned RenderTargetGame = 1;
		const static unsigned RenderTargetMiddle1 = 2;
		const static unsigned RenderTargetMiddle2 = 3;
		const static unsigned RenderTargetMiddle3 = 4;
		const static unsigned RenderTargetMiddle4 = 5;
		const static unsigned RenderTargetMiddle5 = 6;
		const static unsigned RenderTargetUI = 7;
		const static unsigned RenderTargetUI2 = 8;
    	const static unsigned RenderTargetPosteriori = 9;
		const static unsigned RenderTargetPosterioriEngine = 10;
		const static unsigned RenderTargetDummy = 11;
		#define RenderTargetNumber 12

		#ifdef _WIN32
			const static String niteCurrentPlatform = "WINDOWS";
		#elif defined(linux)
			const static String niteCurrentPlatform = "LINUX";
		#else
			const static String niteCurrentPlatform = "UNSUPPORTED OS";
		#endif

		const static String DefaultFontPath = "data/font/SpaceMono-Regular.ttf";

		struct Renderable;
		typedef void (*RenderFunction)(Renderable *object);
		struct Uniform {
				String shaderName;
				Dict<String, float> floats;
				Dict<String, int> integers;
				Dict<String, nite::Vec2> vectors;
				Dict<String, nite::Color> colors;
				void add(String name, nite::Color c);
				void add(String name, nite::Vec2 v);
				void add(String name, float n);
				void add(String name, int n);
				void remove(String name);
				void clear();
		};
		class Shader;
		struct Program {
			String shaderName;
			int id;
			Uniform uniforms;
			nite::Shader *ref;
			~Program();
			Program(int id, Uniform uniforms, const String &name, nite::Shader *ref){
				this->id = id;
				this->uniforms = uniforms;
				this->shaderName = name;
				this->ref = ref;
			}
			Program(int id, const String &name, nite::Shader *ref){
				this->id = id;
				this->shaderName = name;
				this->ref = ref;
			}
		};
		struct Shader;
		struct Renderable {
			nite::Color color;
			nite::Vec2 position;
			nite::RenderFunction function;
			Vector<Program*> programs;
			void apply(nite::Shader &shader);
			void apply(nite::Shader &shader, Uniform uniforms);
			virtual ~Renderable();
			unsigned target;
			int depth;
		};
		class Batch;
		struct RenderableBatch : public nite::Renderable {
			nite::Vec2 origSize;
			nite::Vec2 size;
			nite::Vec2 origin;
			nite::Rect region;
			int objectId;
			float angle;
			bool smooth;
			nite::Vec2 scale;
			nite::Batch *orig;
		};
		bool isItOnBatch();
		class Batch {
			public:
				Vector<Renderable*> objects;
				void begin();
				void end();
				Batch();
				Batch(int w, int h);
				Batch(float w, float h);
				Batch(const nite::Vec2 &size);
				void init(int width, int height);
				~Batch();
				void unload();
				void add(nite::Renderable *ren);
				RenderableBatch *draw(float x, float y);
				RenderableBatch *draw(float x, float y, float angle);
				RenderableBatch *draw(const nite::Vec2 &P);
				RenderableBatch *draw(const nite::Vec2 &P, float angle);
				RenderableBatch *draw(const nite::Vec2 &P, const nite::Vec2 &S, const nite::Vec2 &orig, float angle);
				RenderableBatch *draw(float x, float y, float w, float h, float origX, float origY, float angle);
				Batch& operator= (const Batch &other);				
				void flush();
			private:
				void clearBuffer();
				int objectId;
				nite::Vec2 size;
				bool smooth;
				nite::Vec2 scale;
				nite::Rect region;
		};
		float getTimescale();
		void setTimescale(float ts);
		UInt64 getDelta();
		void setParameters(Vector<String> &params);
		const Vector<String> &getParameters();
		nite::Vec2 getRenderOffset();
		unsigned getWidth();
		unsigned getHeight();
		unsigned getAdjustedWidth();
		unsigned getAdjustedHeight();
		nite::Vec2 getAdjustedSize();
		nite::Vec2 getSize();
		void setZoom(unsigned T, float Z);
		void setZoom(unsigned T, float Z, float mu);
		void setZoomDiscrete(unsigned T, float Z);
		void setAngle(unsigned T, float Z);
		void addRenderList(Renderable *object);
		void dropFrame();
		void graphicsInit();
		void graphicsEnd();
		bool isGraphicsInit();
		void graphicsRender();
		float getGeneralScale();
		void setColor(const nite::Color &color);
		void setColor(float R, float G, float B, float A);
		void setAlpha(float A);
		void setDepth(int D);
		int getDepth();
		void setRenderTarget(unsigned T);
		unsigned getRenderTargetTextureId(unsigned T);
		unsigned getRenderTarget();
		void resetColor();
		nite::Color &getColor();
	}
#endif
