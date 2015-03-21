TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
LIBS += -lpthread
SOURCES += main.cpp
QMAKE_CXXFLAGS += -std=c++1z
include(deployment.pri)
qtcAddDeployment()

HEADERS += \
    cube.hpp

