cmake_minimum_required(VERSION 2.6)

project(LID_PRO CXX C)

set(CMAKE_C_FLAGS "-DGLEW_STATIC -g -m64")
set(CMAKE_CXX_FLAGS "-DGLEW_STATIC -std=c++11 -g -m64")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/builds/cmake)

# Build FreeType2 #
set (FreeType2_src
	src/Engine/FreeType/src/autofit/autofit.c
	src/Engine/FreeType/src/base/ftbase.c
	src/Engine/FreeType/src/base/ftbbox.c
	src/Engine/FreeType/src/base/ftbdf.c
	src/Engine/FreeType/src/base/ftbitmap.c
	src/Engine/FreeType/src/base/ftcid.c
	src/Engine/FreeType/src/base/ftfntfmt.c
	src/Engine/FreeType/src/base/ftfstype.c
	src/Engine/FreeType/src/base/ftgasp.c
	src/Engine/FreeType/src/base/ftglyph.c
	src/Engine/FreeType/src/base/ftgxval.c
	src/Engine/FreeType/src/base/ftinit.c
	src/Engine/FreeType/src/base/ftlcdfil.c
	src/Engine/FreeType/src/base/ftmm.c
	src/Engine/FreeType/src/base/ftotval.c
	src/Engine/FreeType/src/base/ftpatent.c
	src/Engine/FreeType/src/base/ftpfr.c
	src/Engine/FreeType/src/base/ftstroke.c
	src/Engine/FreeType/src/base/ftsynth.c
	src/Engine/FreeType/src/base/ftsystem.c
	src/Engine/FreeType/src/base/fttype1.c
	src/Engine/FreeType/src/base/ftwinfnt.c
	src/Engine/FreeType/src/bdf/bdf.c
	src/Engine/FreeType/src/bzip2/ftbzip2.c
	src/Engine/FreeType/src/cache/ftcache.c
	src/Engine/FreeType/src/cff/cff.c
	src/Engine/FreeType/src/cid/type1cid.c
	src/Engine/FreeType/src/gzip/ftgzip.c
	src/Engine/FreeType/src/lzw/ftlzw.c
	src/Engine/FreeType/src/pcf/pcf.c
	src/Engine/FreeType/src/pfr/pfr.c
	src/Engine/FreeType/src/psaux/psaux.c
	src/Engine/FreeType/src/pshinter/pshinter.c
	src/Engine/FreeType/src/psnames/psnames.c
	src/Engine/FreeType/src/raster/raster.c
	src/Engine/FreeType/src/sfnt/sfnt.c
	src/Engine/FreeType/src/smooth/smooth.c
	src/Engine/FreeType/src/truetype/truetype.c
	src/Engine/FreeType/src/type1/type1.c
	src/Engine/FreeType/src/type42/type42.c
	src/Engine/FreeType/src/winfonts/winfnt.c)

if (WIN32)
  set(FreeType2_src ${FreeType2_src} src/Engine/FreeType/builds/windows/ftdebug.c)
elseif (WINCE)
  set(FreeType2_src ${FreeType2_src} src/Engine/FreeType/builds/wince/ftdebug.c)
else ()
  set(FreeType2_src ${FreeType2_src} src/Engine/FreeType/src/base/ftdebug.c)
endif ()

set(VERSION_MAJOR "2")
set(VERSION_MINOR "9")
set(VERSION_PATCH "0")

add_definitions(-DFT2_BUILD_LIBRARY)

# Find dependencies
foreach (d ZLIB BZip2 PNG HarfBuzz)
  string(TOUPPER "${d}" D)

  if (DEFINED WITH_${d} OR DEFINED WITH_${D})
    if (WITH_${d} OR WITH_${D})
      find_package(${d} QUIET REQUIRED)
    endif ()
  else ()
    find_package(${d} QUIET)
  endif ()

  if (${d}_FOUND OR ${D}_FOUND)
    message(STATUS "Building with ${d}")
  endif ()
endforeach ()

if (UNIX)

  file(READ "src/Engine/FreeType/builds/unix/ftconfig.in"
    FTCONFIG_H)
  if (HAVE_UNISTD_H)
    string(REGEX REPLACE
      "#undef +(HAVE_UNISTD_H)" "#define \\1"
      FTCONFIG_H "${FTCONFIG_H}")
  endif ()
  if (HAVE_FCNTL_H)
    string(REGEX REPLACE
      "#undef +(HAVE_FCNTL_H)" "#define \\1"
      FTCONFIG_H "${FTCONFIG_H}")
  endif ()
  if (HAVE_STDINT_H)
    string(REGEX REPLACE
      "#undef +(HAVE_STDINT_H)" "#define \\1"
      FTCONFIG_H "${FTCONFIG_H}")
  endif ()
  string(REPLACE "/undef " "#undef "
    FTCONFIG_H "${FTCONFIG_H}")
  file(WRITE "src/Engine/FreeType/include/freetype/config/ftconfig.h-new"
    "${FTCONFIG_H}")
else ()
  file(READ "src/Engine/FreeType/include/freetype/config/ftconfig.h"
    FTCONFIG_H)
  file(WRITE "src/Engine/FreeType/include/freetype/config/ftconfig.h-new"
    "${FTCONFIG_H}")
endif ()

execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different
 	src/Engine/FreeType/include/freetype/config/ftconfig.h-new"
	src/Engine/FreeType/include/freetype/config/ftconfig.h")

include_directories("src/Engine/FreeType/include")
include_directories(BEFORE "src/Engine/FreeType/include")	


add_library(FreeType2 ${FreeType2_src})

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
	# RING #
	src/RING/RING.cpp
	src/RING/Search.cpp
	src/RING/Blueprint.cpp
	# Core #
	src/Core/Camera.cpp
	src/Core/Server.cpp
	src/Core/Client.cpp
	src/Core/Network.cpp
	src/Core/Object.cpp
	src/Core/World.cpp
	src/Core/Input.cpp
	# Entity #
	src/Entity/Anim.cpp
	src/Entity/Inventory.cpp
	src/Entity/Base.cpp
	src/Entity/Battle.cpp
	src/Entity/Stat.cpp
	src/Entity/Skill.cpp
	src/Entity/Effect.cpp
	# UI #
	src/UI/HUD.cpp
	src/UI/Icon.cpp
	src/UI/IngameMenu.cpp
	# GENERAL #
	src/AI/AI.cpp
	src/Game.cpp
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
target_link_libraries(src FreeType2)

IF(MINGW)
	target_link_libraries(src RaptorAudio)
ENDIF(MINGW)

aux_source_directory(src/ SRC_LIST)

add_executable(Game src/LID.cpp)
#add_executable(Dedicated src/Server.cpp)#

target_link_libraries(Game src)
#target_link_libraries(Dedicated src)#