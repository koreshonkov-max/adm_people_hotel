TEMPLATE = app
TARGET = adm_people_hotel
CONFIG += console c++17
CONFIG -= qt app_bundle
win32-msvc*: QMAKE_CXXFLAGS += /utf-8

SOURCES += main.cpp guestbook.cpp
HEADERS += guest.h guestbook.h
