dnl
dnl   /**------- <| --------------------------------------------------------**
dnl    **         A                     Clan                                **
dnl    **---     /.\   -----------------------------------------------------**
dnl    **   <|  [""M#               configure.in                            **
dnl    **-   A   | #   -----------------------------------------------------**
dnl    **   /.\ [""M#         First version: 30/04/2008                     **
dnl    **- [""M# | #  U"U#U  -----------------------------------------------**
dnl         | #  | #  \ .:/
dnl         | #  | #___| #
dnl ******  | "--'     .-"  ***************************************************
dnl *     |"-"-"-"-"-#-#-##   Clan : the Chunky Loop Analyser (experimental)  *
dnl ****  |     # ## ######  **************************************************
dnl *      \       .::::'/                                                    *
dnl *       \      ::::'/     Copyright (C) 2008 University Paris-Sud 11      *
dnl *     :8a|    # # ##                                                      *
dnl *     ::88a      ###      This is free software; you can redistribute it  *
dnl *    ::::888a  8a ##::.   and/or modify it under the terms of the GNU     *
dnl *  ::::::::888a88a[]:::   Lesser General Public License as published by   *
dnl *::8:::::::::SUNDOGa8a::. the Free Software Foundation, either version    *
dnl *::::::::8::::888:Y8888::                2.1 of the License, or (at your  *
dnl *::::':::88::::888::Y88a::::::::::::...  option) any later version.       *
dnl *::'::..    .   .....   ..   ...  .                                       *
dnl * This software is distributed in the hope that it will be useful, but    *
dnl * WITHOUT ANY WARRANTY; without even the implied warranty of              *
dnl * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
dnl * General Public License  for more details.	                              *
dnl *                                                                         *
dnl * You should have received a copy of the GNU Lesser General Public        *
dnl * License along with software; if not, write to the Free Software         *
dnl * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307    *
dnl * USA                                                                     *
dnl *                                                                         *
dnl * Clan, the Chunky Loop Analyser                                          *
dnl * Written by Cedric Bastoul, Cedric.Bastoul@u-psud.fr                     *
dnl *                                                                         *
dnl ***************************************************************************/
dnl
dnl Input file for autoconf to build a configuration shellscript.

m4_define([version_major], [0])
m4_define([version_minor], [8])
m4_define([version_revision], [0])

AC_PREREQ(2.53)

dnl AC_PROG_SED is only available from version 2.6 (released in 2003). CentosOS
dnl 5.10 still has an ancient version. The actual AC_PROG_SED macro does
dnl functional checks, but here
dnl we define a trivial local version for times when we are running on
dnl obsoldete autoconf.
ifdef([AC_PROG_SED], [], [
  AC_DEFUN([AC_PROG_SED], [
    AC_CHECK_PROG(SED, sed, sed)
  ])
])

dnl Fill here the @bug email adress.
AC_INIT([clan], [version_major.version_minor.version_revision],
        [cedric.bastoul@u-psud.fr,pouchet@cse.ohio-state.edu])
dnl A common file, which serve as a test.
AC_CONFIG_SRCDIR([main.cpp])
dnl Put as most as possible configuration files to an auxialiry
dnl directory.
AC_CONFIG_AUX_DIR(autoconf)
AC_CONFIG_MACRO_DIR([m4])

dnl Initialize automake.
AM_INIT_AUTOMAKE([foreign])
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

debug="false"
dnl debugging flag
AC_ARG_ENABLE(debug,
              [  --enable-debug  enable compilation with debugging info ],
              debug="true")

AM_CONDITIONAL(PLUTO_CODEGEN_CXX_DEBUG, test x$debug = xtrue)



dnl /**************************************************************************
dnl  *                              Checking                                  *
dnl  **************************************************************************/


dnl Checks for programs.
AC_PROG_CXX
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_CHECK_PROG(CD, cd)
AC_PROG_INSTALL
AC_PROG_LIBTOOL

dnl /**************************************************************************
dnl  *                             Option setting                             *
dnl  **************************************************************************/


dnl /**************************************************************************
dnl  *                            Substitutions                               *
dnl  **************************************************************************/


dnl Substitutions to do.
AC_SUBST(ac_aux_dir)
AC_SUBST(abs_top_srcdir)

AC_SUBST(VERSION_MAJOR)
AC_SUBST(VERSION_MINOR)
AC_SUBST(VERSION_REVISION)

dnl Configure Makefiles.
AC_CONFIG_FILES([
	Makefile
	],
	[test -z "$CONFIG_HEADERS" || echo timestamp > source/stamp-h.in])

AC_OUTPUT

echo "             /*-----------------------------------------------*"
echo "              *   pluto_codegen_cxx configuration is OK       *"
echo "              *-----------------------------------------------*/"
echo "It appears that your system is OK to start pluto codegen cxx compilation. You need"
echo "now to type \"make\". Lastly type \"make install\" to install Clan on"
echo "your system (log as root if necessary)."
