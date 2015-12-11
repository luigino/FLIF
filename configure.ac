#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([FLIF], [0.2rc2], [jonsneyers@gmail.com])
AM_INIT_AUTOMAKE([1.11])
LT_INIT
#AC_SUBST([LIBTOOL_DEPS])
AC_CONFIG_SRCDIR([Makefile.am])

# Checks for programs.
AC_PROG_CXX
#AM_CONDITIONAL(COMPILER_IS_GCC, [test x"$CXX" = x"g++"])
AC_PROG_CC
AC_PROG_LN_S

# Checks for header files.
AC_CHECK_HEADERS([stddef.h stdint.h stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_INT8_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_CHECK_FUNCS([memset select strcasecmp strrchr strtol])

PKG_PROG_PKG_CONFIG()

dnl ==============================================================
dnl png lib
dnl ==============================================================
PKG_CHECK_MODULES([LIBPNG], [libpng], [:],
  AC_MSG_ERROR([*** Libpng development files not found!]))

dnl ==============================================================
dnl sdl2 lib
dnl ==============================================================
PKG_CHECK_MODULES([SDL2], [sdl2], [:],
  AC_MSG_ERROR([*** Libsdl2 development files not found!]))

dnl ==============================================================
dnl zlib
dnl ==============================================================
PKG_CHECK_MODULES([ZLIB], [zlib], [:],
  AC_MSG_ERROR([*** zlib development files not found!]))

AC_ARG_ENABLE([magic],
              [AS_HELP_STRING([--disable-magic],
                              [Don't add file signature to magic file])],
              [],
              [enable_magic=yes])

OPTIMIZATIONS = "-std=gnu++11 -DNDEBUG -O3 -ftree-vectorize -g0 -Wall"
LIB_OPTIMIZATIONS = "-DNDEBUG -O3"
if test x"$CXX" = x"g++"; then
   OPTIMIZATIONS = "$OPTIMIZATIONS -flto -fwhole-program"
   LIB_OPTIMIZATIONS = "$LIB_OPTIMIZATIONS -flto"
fi

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
