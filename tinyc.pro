TEMPLATE = app
#TARGET = tinycmd
TARGET = tinytest
CONFIG = c99 link_pkgconfig
PKGCONFIG =
DEFINES = _GNU_SOURCE __STDC_FORMAT_MACROS bool=BOOL true=TRUE false=FALSE
INCLUDEPATH = lib

HEADERS = \
    lib/cdirent.h \
    lib/cdirparser.h \
    lib/cfile.h \
    lib/cfileinfo.h \
    lib/cinifile.h \
    lib/clist.h \
    lib/cprocess.h \
    lib/cregexp.h \
    lib/cstring.h \
    lib/cstringlist.h \
    lib/libapp.h \
    lib/libconv.h \
    lib/libhtml.h \
    lib/libmacros.h \
    lib/libpath.h \
    lib/libstr.h \
    lib/libtest.h \
    lib/print.h \
    test_main.h \

SOURCES = \
    lib/cdirent.c \
    lib/cdirparser.c \
    lib/cfile.c \
    lib/cfileinfo.c \
    lib/cinifile.c \
    lib/clist.c \
    lib/cprocess.c \
    lib/cregexp.c \
    lib/cstring.c \
    lib/cstringlist.c \
    lib/libapp.c \
    lib/libconv.c \
    lib/libhtml.c \
    lib/libpath.c \
    lib/libstr.c \
    lib/libtest.c \
    lib/print.c \
    0Temp.c \
    main.c \
    test/test01_cstring.c \
    test/test02_cstringlist.c \
    test/test03_clist.c \
    test/test04_cfile.c \
    test/test05_cfileinfo.c \
    test/test06_cdirent.c \
    test/test07_cdirparser.c \
    test/test08_cinifile.c \
    test/test09_cprocess.c \
    test/test10_libapp.c \
    test/test11_libhtml.c \
    test/test12_libpath.c \
    test/test13_libstr.c \
    test_main.c \

DISTFILES = \
    install.sh \
    meson.build \
    Readme.md \


