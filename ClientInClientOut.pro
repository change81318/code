QT += core
QT -= gui
QT += network

TARGET = ClientInClientOut
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    clientinclientout.cpp

HEADERS += \
    clientinclientout.h

