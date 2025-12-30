TEMPLATE = app
TARGET = testcmd
CONFIG = c99 link_pkgconfig
DEFINES = _GNU_SOURCE bool=BOOL true=TRUE false=FALSE _LINUX_
INCLUDEPATH = ../i2c
PKGCONFIG =

#PKGCONFIG += tinyc

HEADERS = \
    mcp9808.h

SOURCES = \
    0temp.c \
    main.c \
    mcp9808.c

DISTFILES = \
    install.sh \
    License.txt \
    meson.build \
    Readme.md \

