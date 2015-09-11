
TEMPLATE = app

VPATH += src/ui src/sockc
INCLUDEPATH += . src/ui src/sockc /usr/include
DEPENDPATH += . src/ui src/sockc

CONFIG += c++11 thread warn_on
CONFIG(gcc): QMAKE_CXX_LFLAGS += -pthread
CONFIG(debug) {
    QMAKE_CXX_FLAGS += -DCS_DEBUG=2
    posix {
        #QMAKE_CXX = clang++
        QMAKE_CXX_FLAGS += -DCS_DEBUG=2 -O0 -g3 -ferror-limit=5
        QMAKE_CXX_LFLAGS += -rdynamic
    }
} else {
    QMAKE_CXX_FLAGS += -DNDEBUG -O2
    QMAKE_CXX_LFLAGS += -Wl,O2
}

posix {
    MOC_DIR = /tmp
    OBJECTS_DIR = /tmp
}
win32 {
    MOC_DIR = c:/tmp
    OBJECTS_DIR = c:/tmp
}

SOURCES += src/main.cpp

include(src/ui/ui.pri)
include(src/sockc/sockc.pri)
