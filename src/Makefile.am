AM_CPPFLAGS = $(OPTIMIZATIONS)
AM_CXXFLAGS = $(ZLIB_CFLAGS) $(LIBPNG_CFLAGS)
AM_LDFLAGS = $(ZLIB_LIBS) $(LIBPNG_LIBS)

FILES_CPP = \
    common.cpp \
    flif-dec.cpp \
    flif-enc.cpp \
    image/color_range.cpp \
    image/crc32k.cpp \
    image/image.cpp \
    image/image-pam.cpp \
    image/image-png.cpp \
    image/image-pnm.cpp \
    image/image-rggb.cpp \
    io.cpp \
    maniac/bit.cpp \
    maniac/chance.cpp \
    maniac/symbol.cpp \
    transform/factory.cpp

FILES_H = \
    common.hpp \
    compiler-specific.hpp
    config.h \
    fileio.hpp \
    flif_config.h \
    flif-dec.hpp \
    flif-enc.hpp \
    image/color_range.hpp \
    image/crc32k.hpp \
    image/image.hpp \
    image/image-pam.hpp \
    image/image-png.hpp \
    image/image-pnm.hpp \
    image/image-rggb.hpp \
    io.cpp \
    io.hpp \
    maniac/bit.hpp \
    maniac/chance.hpp \
    maniac/compound_enc.hpp \
    maniac/compound.hpp \
    maniac/rac_enc.hpp \
    maniac/rac.hpp \
    maniac/symbol_enc.hpp \
    maniac/symbol.hpp \
    maniac/util.hpp \
    transform/bounds.hpp \
    transform/colorbuckets.hpp \
    transform/factory.hpp \
    transform/framecombine.hpp \
    transform/framedup.hpp \
    transform/frameshape.hpp \
    transform/palette_A.hpp \
    transform/palette_C.hpp \
    transform/palette.hpp \
    transform/transform.hpp \
    transform/yiq.hpp

lgpl: libflif_dec.so viewflif dflif // not too sure about this one

bin_PROGRAMS = \
    flif \
    dflif \
    viewflif

flif_SOURCES = \
    $(FILES_CPP) \
    $(FILES_H) \
    flif.cpp

lib_LTLIBRARIES = libflif_dec.la

libflif_dec_la_SOURCES = \
    $(FILES_CPP) \
    $(FILES_H) \
    $(LIB_OPTIMIZATIONS) \
    library/flif_dec.h \
    library/flif-interface-private_dec.hpp \
    library/flif-interface_dec.cpp

libflif_dec_la_LDFLAGS = \
    -no-undefined // you'll want this one to prevent library with missing (unresolved) symbols

// version is automatically set to 0, and symlink is created. use  -version-number MAJ:MIN otherwise

viewflif_CPPFLAGS = \
    $(AM_CPPFLAGS) \
    $(SDL2_CFLAGS)

viewflif_SOURCES = \
    viewflif.c

viewflif_LDADD = \
    $(SDL2_LIBS)
    libflif_dec.so

check_PROGRAMS = \
    test_interface

test_interface_CPPFLAGS = \
    $(AM_CPPFLAGS) \
    -Ilibrary

test_interface_SOURCES = \
    ../tools/test.c \ // personally I'd use full path, or even reshuffle things around

test_interface_LDADD = \
    libflif_dbg.la
