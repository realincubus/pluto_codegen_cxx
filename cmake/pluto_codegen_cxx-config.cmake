# Try to find the pluto library

# PLUTO_CODEGEN_CXX_FOUND       - System has isl lib
# PLUTO_CODEGEN_CXX_INCLUDE_DIR - The isl include directory
# PLUTO_CODEGEN_CXX_LIBRARY     - Library needed to use isl


if (PLUTO_CODEGEN_CXX_INCLUDE_DIR AND PLUTO_CODEGEN_CXX_LIBRARY)
	# Already in cache, be silent
	set(PLUTO_CODEGEN_CXX_FIND_QUIETLY TRUE)
endif()

find_path(PLUTO_CODEGEN_CXX_INCLUDE_DIR NAMES pluto_codegen_cxx/pluto_codegen_cxx.hpp)
find_library(PLUTO_CODEGEN_CXX_LIBRARY NAMES pluto_codegen_cxx)

if (PLUTO_CODEGEN_CXX_LIBRARY AND PLUTO_CODEGEN_CXX_INCLUDE_DIR)
	message(STATUS "Library pluto found =) ${PLUTO_CODEGEN_CXX_LIBRARY}")
else()
	message(STATUS "Library pluto not found =(")
endif()

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(PLUTO_CODEGEN_CXX DEFAULT_MSG PLUTO_CODEGEN_CXX_INCLUDE_DIR PLUTO_CODEGEN_CXX_LIBRARY)

mark_as_advanced(PLUTO_CODEGEN_CXX_INCLUDE_DIR PLUTO_CODEGEN_CXX_LIBRARY)
