# pluto_codegen_cxx
#
# Copyright (C) Stefan Kemnitz 2016
#
# This library is made to allow the construction of the replacement file fully in memory ( stringstream )
# this way one can execute the code generation in parallel

SUBDIRS = 

MAINTAINERCLEANFILES = Makefile.in

if PLUTO_CODEGEN_CXX_DEBUG
OPT_FLAGS = 
DEBUG_FLAGS = -g
else
OPT_FLAGS = -O3 -funroll-loops -fopenmp
endif

lib_LTLIBRARIES = libpluto_codegen_cxx.la

pluto_codegen_cxxdir=$(includedir)/pluto_codegen_cxx

pluto_codegen_cxx_HEADERS=\
	include/pluto_codegen_cxx.hpp\
	include/pluto_cxx.hpp

libpluto_codegen_cxx_la_SOURCES = main.cpp statement_information.cpp codegen.cpp codegen_omp.cpp codegen_acc.cpp codegen_cilk.cpp codegen_tbb.cpp codegen_hpx.cpp codegen_cuda.cpp pluto_cxx.cpp
libpluto_codegen_cxx_la_CPPFLAGS = -std=c++1y $(OPT_FLAGS) $(DEBUG_FLAGS) \
   -DSCOPLIB_INT_T_IS_LONGLONG -DCLOOG_INT_GMP \
   -I include \
   -I../include \
   -I../cloog-isl/include \
   -I../openscop/include \
   -I../candl/include \
   -I../piplib/include \
   -I../clan/include \
   -I../isl/include \
   -I../src/ \
   $(ISL_INCLUDE)

libpluto_codegen_cxx_la_LIBADD = \
   ../src/libpluto.la \
   ../cloog-isl/libcloog-isl.la \
   $(ISL_LIBADD)

EXTRA_DIST = \
	cmake

install-data-local:
	@test -z "$(DESTDIR)$(libdir)/pluto_codegen_cxx" || $(mkdir_p) "$(DESTDIR)$(libdir)/pluto_codegen_cxx"
	$(INSTALL_DATA) "$(top_srcdir)/cmake/pluto_codegen_cxx-config.cmake" "$(DESTDIR)$(libdir)/pluto_codegen_cxx/"
	
uninstall-local:
	rm -f "$(DESTDIR)$(libdir)/pluto_codegen_cxx/pluto_codegen_cxx-config.cmake"

total:
	@echo "             /*-----------------------------------------------*"
	@echo "              *                     pluto_codegen_cxx         *"
	@echo "              *-----------------------------------------------*/"
	$(MAKE) uninstall
	$(MAKE) clean
	$(MAKE)
	$(MAKE) install
