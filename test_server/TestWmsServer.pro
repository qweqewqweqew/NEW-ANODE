QT += core httpserver network
QT -= gui

CONFIG += c++17 console
CONFIG -= app_bundle

TARGET = TestWmsServer
TEMPLATE = app

SOURCES += \
    main.cpp \
    testwmsserver.cpp

HEADERS += \
    testwmsserver.h

# 默认构建目录
DESTDIR = ../bin























