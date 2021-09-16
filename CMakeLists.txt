cmake_minimum_required(VERSION 2.6)

project(REDBIAS_PRO CXX C)

set(CMAKE_C_FLAGS "-DGLEW_STATIC -g -m64")
set(CMAKE_CXX_FLAGS "-DGLEW_STATIC -std=c++11 -g -m64")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/builds/cmake)

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
	src/Engine/Tools/hashids.cpp
	src/Engine/Input.cpp
	src/Engine/Graphics.cpp
	src/Engine/Types.cpp
	src/Engine/JSON/Jzon.cpp
	src/Engine/Tools/Files.cpp
	src/Engine/Tools/Hash.cpp
	src/Engine/Tools/StepTimer.cpp
	src/Engine/Tools/Log.cpp
	src/Engine/Tools/Math.cpp
	src/Engine/Tools/AsyncTask.cpp
	src/Engine/Tools/String.cpp
	src/Engine/Tools/System.cpp
	src/Engine/Tools/Time.cpp
	src/Engine/Tools/MD5.cpp
	src/Engine/Tools/SmallPacket.cpp
	src/Engine/Shader.cpp
	src/Engine/Tileset.cpp
	src/Engine/Packets.cpp
	src/Engine/Network.cpp
	src/Engine/View.cpp
	src/Engine/Graphics.cpp
	src/Engine/Shapes.cpp
	src/Engine/Texture.cpp
	src/Engine/Font.cpp
	src/Engine/Particles.cpp
	src/Engine/Animation.cpp
	src/Engine/Indexer.cpp
	src/Engine/Map.cpp
	src/Engine/Object.cpp
	src/Engine/nScript.cpp
	src/Engine/Audio.cpp
	src/Engine/UI/UI.cpp
	src/Engine/UI/Custom/ProgressiveBarUI.cpp
	src/Engine/UI/WindowUI.cpp
	src/Engine/UI/PanelUI.cpp
	src/Engine/UI/TextUI.cpp
	src/Engine/UI/IconUI.cpp
	src/Engine/UI/ButtonUI.cpp
	src/Engine/UI/CheckboxUI.cpp
	src/Engine/UI/BaseUIComponent.cpp
	src/Engine/Console.cpp		)

add_library(nite ${nite_src})

set(src_files
	src/UI/BattlePlayerInfo.cpp
	src/UI/DialogBox.cpp
	src/World.cpp
	src/Battle.cpp
	src/Entity.cpp
	src/Vfx.cpp
	src/Object.cpp
	src/Dialog.cpp
)
add_library(src ${src_files})
target_link_libraries(src nite)


target_link_libraries(src GLEW)

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
#target_link_libraries(src FreeType2)#
target_link_libraries(src -lfreetype)

IF(MINGW)
	target_link_libraries(src RaptorAudio)
ENDIF(MINGW)

aux_source_directory(src/ SRC_LIST)

add_executable(Game src/Game.cpp)
#add_executable(Dedicated src/Server.cpp)#

target_link_libraries(Game src)
#target_link_libraries(Dedicated src)#