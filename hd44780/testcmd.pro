TEMPLATE = app
TARGET = testcmd
CONFIG = c99 link_pkgconfig
DEFINES = _GNU_SOURCE bool=BOOL true=TRUE false=FALSE _LINUX_
INCLUDEPATH =
PKGCONFIG =

PKGCONFIG += tinyc

HEADERS = \
    hd44780.h \

SOURCES = \
    0temp.c \
    main.c \
    hd44780.c \

DISTFILES = \
    install.sh \
    License.txt \
    meson.build \
    Readme.md \

