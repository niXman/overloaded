TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXXFLAGS += \
    -std=c++11

SOURCES += \
	main.cpp

INCLUDEPATH += \
    include

HEADERS += \
    include/overloaded_function.hpp
