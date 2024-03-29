# - Find vorbis
# Find the native vorbis includes and libraries
#
#  VORBIS_INCLUDE_DIR - where to find vorbis.h, etc.
#  VORBIS_LIBRARIES   - List of libraries when using vorbis(file).
#  VORBIS_FOUND       - True if vorbis found.

# Taken from the allegro project, licensed under zlib.
# https://raw.githubusercontent.com/liballeg/allegro5/master/cmake/FindOgg.cmake
# https://github.com/liballeg/allegro5/blob/master/LICENSE.txt

if(VORBIS_INCLUDE_DIR)
    # Already in cache, be silent
    set(VORBIS_FIND_QUIETLY TRUE)
endif(VORBIS_INCLUDE_DIR)

find_package(Ogg)
if(OGG_FOUND)
	find_path(VORBIS_INCLUDE_DIR vorbis/vorbisfile.h)
	# MSVC built vorbis may be named vorbis_static
	# The provided project files name the library with the lib prefix.
	find_library(VORBIS_LIBRARY
	    NAMES vorbis vorbis_static libvorbis libvorbis_static)
	find_library(VORBISFILE_LIBRARY
	    NAMES vorbisfile vorbisfile_static libvorbisfile libvorbisfile_static)
	# Handle the QUIETLY and REQUIRED arguments and set VORBIS_FOUND
	# to TRUE if all listed variables are TRUE.
	include(FindPackageHandleStandardArgs)
	find_package_handle_standard_args(VORBIS DEFAULT_MSG
	    VORBIS_INCLUDE_DIR
	    VORBIS_LIBRARY VORBISFILE_LIBRARY)
endif(OGG_FOUND)

if(VORBIS_FOUND)
  set(VORBIS_LIBRARIES ${VORBISFILE_LIBRARY} ${VORBIS_LIBRARY}
        ${OGG_LIBRARY})
else(VORBIS_FOUND)
  set(VORBIS_LIBRARIES)
endif(VORBIS_FOUND)

mark_as_advanced(VORBIS_INCLUDE_DIR)
mark_as_advanced(VORBIS_LIBRARY VORBISFILE_LIBRARY)

