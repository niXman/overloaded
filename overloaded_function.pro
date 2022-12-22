TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

#QMAKE_CXX = clang++-14

QMAKE_CXXFLAGS += \
    -std=c++11

SOURCES += \
	main.cpp

HEADERS += \
    overloaded_function.hpp
