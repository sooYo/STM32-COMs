TEMPLATE = app
CONFIG += console
CONFIG -= qt

SOURCES += main.cpp \
    w25q16.c \
    SpiFile.cpp

HEADERS += \
    w25q16.h \
    SpiFile.h

