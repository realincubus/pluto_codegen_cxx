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

# libpluto doesn't use polylib-based functions
libpluto_codegen_cxx_la_SOURCES = main.cpp clast_acc.cpp clast_omp.cpp
libpluto_codegen_cxx_la_CPPFLAGS = -std=c++1y $(OPT_FLAGS) $(DEBUG_FLAGS) \
   -DSCOPLIB_INT_T_IS_LONGLONG -DCLOOG_INT_GMP \
   -I include \
   -I../include \
   -I../piplib/include \
   -I../clan/include \
   -I../openscop/include \
   -I../candl/include \
   -I../cloog-isl/include \
   -I../src/ \
   $(ISL_INCLUDE)

# libpluto doesn't use polylib
libpluto_codegen_cxx_la_LIBADD = \
   ../src/libpluto.la \
   ../cloog-isl/libcloog-isl.la \
   $(ISL_LIBADD)
