cmake_minimum_required(VERSION 2.6)

project(MMORPG CXX C)

include_directories("C:\\Users\\irusso\\Documents\\MinGW\\include")

set(CMAKE_C_FLAGS "-DGLEW_STATIC -g")
set(CMAKE_CXX_FLAGS "-DGLEW_STATIC -std=c++11 -g")

# Build SOIL #
set (SOIL_src
		src/Engine/SOIL/image_DXT.c
		src/Engine/SOIL/image_helper.c
		src/Engine/SOIL/SOIL.c
		src/Engine/SOIL/stb_image_aug.c)

add_library(SOIL ${SOIL_src})

IF(MINGW)
	target_link_libraries(SOIL -lopengl32)
ELSE(MINGW)
	target_link_libraries(SOIL -lGL)
ENDIF(MINGW)


# Build GLEW #
set (GLEW_src
		src/Engine/GLEW/glew.c)

add_library(GLEW ${GLEW_src})

IF(MINGW)
	# Build RaptorAudio #
	set (RaptorAudio_src
			src/Engine/RaptorAudio/stb_vorbis.c
			src/Engine/RaptorAudio/AudioSource.cpp
			src/Engine/RaptorAudio/AudioSourceFile.cpp
			src/Engine/RaptorAudio/AudioSourceFile.cpp
			src/Engine/RaptorAudio/AudioSourceMemory.cpp
			src/Engine/RaptorAudio/BlockBuffer.cpp
			src/Engine/RaptorAudio/DSPChain.cpp
			src/Engine/RaptorAudio/DSPVariables.cpp
			src/Engine/RaptorAudio/HistoryBufferObject.cpp
			src/Engine/RaptorAudio/MemorySoundObject.cpp
			src/Engine/RaptorAudio/MemorySoundObjectWavImpl.cpp
			src/Engine/RaptorAudio/OggDecoder.cpp
			src/Engine/RaptorAudio/RingBuffer.cpp
			src/Engine/RaptorAudio/SharedPointer.cpp
			src/Engine/RaptorAudio/SoundMixer.cpp
			src/Engine/RaptorAudio/SoundObject.cpp
			src/Engine/RaptorAudio/StreamingSoundObject.cpp
			src/Engine/RaptorAudio/StreamingSoundObjectOggImpl.cpp
			src/Engine/RaptorAudio/StreamingSoundObjectWavImpl.cpp
			src/Engine/RaptorAudio/WaveoutDevice.cpp)

	add_library(RaptorAudio ${RaptorAudio_src})
	target_link_libraries(RaptorAudio -lwinmm)
ENDIF(MINGW)

# Build ninte #
set (nite_src
		src/Engine/Input.cpp
		src/Engine/Graphics.cpp
		src/Engine/Types.cpp
		src/Engine/JSON/Jzon.cpp
		src/Engine/Tools/Files.cpp
		src/Engine/Tools/Hash.cpp
		src/Engine/Tools/Log.cpp
		src/Engine/Tools/Math.cpp
		src/Engine/Tools/String.cpp
		src/Engine/Tools/System.cpp
		src/Engine/Tools/Time.cpp
		src/Engine/Shader.cpp
		src/Engine/Tileset.cpp
		src/Engine/Packets.cpp
		src/Engine/Network.cpp
		src/Engine/View.cpp
		src/Engine/Graphics.cpp
		src/Engine/Shapes.cpp
		src/Engine/Texture.cpp
		src/Engine/Font.cpp
		src/Engine/Animation.cpp
		src/Engine/World.cpp
		src/Engine/World.cpp
		src/Engine/Object.cpp
		src/Engine/Tools/MD5.cpp
		src/Engine/Console.cpp
		src/Engine/Audio.cpp)

add_library(nite ${nite_src})

set(src_files
	src/Entity.cpp
	src/Player.cpp
	src/Inventory.cpp
	src/Mob.cpp
	src/Map.cpp
	src/Sword.cpp
	src/AI/AI.cpp
	
	src/UI/UI.cpp
	src/UI/WindowUI.cpp
	src/UI/PanelUI.cpp
	src/UI/TextUI.cpp
	src/UI/ButtonUI.cpp
	src/UI/CheckboxUI.cpp	
	src/UI/BaseUIComponent.cpp
)
add_library(src ${src_files})
target_link_libraries(src nite)


target_link_libraries(src GLEW)
target_link_libraries(src ${CMAKE_SOURCE_DIR}/freetype.a)

IF(MINGW)
	target_link_libraries(src ${CMAKE_SOURCE_DIR}/freetype.a)
ELSE(MINGW)
	target_link_libraries(src -lfreetype)
ENDIF(MINGW)

IF(MINGW)
	target_link_libraries(src -lmingw32)
ENDIF(MINGW)

target_link_libraries(src -lSDL2)
target_link_libraries(src -lSDL2main)

IF(MINGW)
	target_link_libraries(src -lopengl32)
ELSE(MINGW)
	target_link_libraries(src -lGL)
ENDIF(MINGW)

target_link_libraries(src -lpthread)
IF(MINGW)
	target_link_libraries(src -lws2_32)
ENDIF(MINGW)
target_link_libraries(src SOIL)

IF(MINGW)
	target_link_libraries(src RaptorAudio)
ENDIF(MINGW)

aux_source_directory(src/ SRC_LIST)

add_executable(Game src/Game.cpp)

target_link_libraries(Game src)
