# Try to find the pluto library

# PLUTO_CODEGEN_CXX_FOUND       - System has isl lib
# PLUTO_CODEGEN_CXX_INCLUDE_DIR - The isl include directory
# PLUTO_CODEGEN_CXX_LIBRARY     - Library needed to use isl


if (PLUTO_CODEGEN_CXX_INCLUDE_DIR AND PLUTO_CODEGEN_CXX_LIBRARY)
	# Already in cache, be silent
	set(PLUTO_CODEGEN_CXX_FIND_QUIETLY TRUE)
endif()

find_path(PLUTO_CODEGEN_CXX_INCLUDE_DIR NAMES include/pluto_codegen_cxx/pluto_codegen_cxx.hpp)
find_library(PLUTO_CODEGEN_CXX_LIBRARY NAMES lib/libpluto_codegen_cxx.so)
find_library(PLUTO_CODEGEN_CXX_STATIC_LIBRARY NAMES lib/libpluto_codegen_cxx.a)
find_library(PLUTO_STATIC_LIBRARY NAMES lib/libpluto.a)
find_library(ISL_LIBRARY NAMES lib/libisl.a)
find_library(CLOOG_ISL_LIBRARY NAMES lib/libcloog-isl.a)

set ( PLUTO_CODEGEN_CXX_INCLUDE_DIR "${PLUTO_CODEGEN_CXX_INCLUDE_DIR}/pluto_codegen_cxx/" )

if (PLUTO_CODEGEN_CXX_LIBRARY AND PLUTO_CODEGEN_CXX_INCLUDE_DIR)
	message(STATUS "Include dir pluto_codegen_cxx found =) ${PLUTO_CODEGEN_CXX_INCLUDE_DIR}")
	message(STATUS "Library pluto_codegen_cxx found =) ${PLUTO_CODEGEN_CXX_LIBRARY}")
	message(STATUS "Library pluto found =) ${PLUTO_STATIC_LIBRARY}")
	message(STATUS "Library cloog-isl found =) ${CLOOG_ISL_LIBRARY}")
	message(STATUS "Library isl found =) ${ISL_LIBRARY}")
else()
  message(STATUS "Include dir pluto_codegen_cxx not found =(")
	message(STATUS "Library pluto_codegen_cxx not found =(")
endif()

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(PLUTO_CODEGEN_CXX DEFAULT_MSG PLUTO_CODEGEN_CXX_INCLUDE_DIR PLUTO_CODEGEN_CXX_LIBRARY)

mark_as_advanced(PLUTO_CODEGEN_CXX_INCLUDE_DIR PLUTO_CODEGEN_CXX_LIBRARY)
