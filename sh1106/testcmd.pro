TEMPLATE = app
TARGET = testcmd
CONFIG = c99 link_pkgconfig
DEFINES = _GNU_SOURCE bool=BOOL true=TRUE false=FALSE _LINUX_
INCLUDEPATH =
PKGCONFIG =

#PKGCONFIG += tinyc

HEADERS += \
    BitBang_I2C.h \
    ss_oled.h \

SOURCES = \
    BitBang_I2C.c \
    main.c \
    ss_oled.c

DISTFILES = \
    install.sh \
    License.txt \
    meson.build \
    Readme.md \

