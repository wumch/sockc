
TEMPLATE = app

VPATH += src/ui src/sockc
INCLUDEPATH += . src/ui src/sockc /bak/android/ndk/sources/boost/1.58.0/include /bak/android/ndk/sources/openssl/include /bak/android/ndk/sources/cryptop++/include
DEPENDPATH += . src/ui src/sockc

CONFIG += c++11 thread warn_on
CONFIG(gcc): QMAKE_CXX_LFLAGS += -pthread
CONFIG(debug) {
    QMAKE_CXX_FLAGS += -DCS_DEBUG=2
    posix {
        #QMAKE_CXX = clang++
        QMAKE_CXXFLAGS += -DCS_DEBUG=2 -O0 -g3
        QMAKE_LFLAGS += -rdynamic
    }
} else {
    QMAKE_CXXFLAGS += -DNDEBUG -O2
    QMAKE_LFLAGS += -Wl,O2
}
QMAKE_LFLAGS +=  -L/bak/android/ndk/platforms/android-15/arch-arm/usr/lib -L/bak/android/ndk/sources/boost/1.58.0/libs/armeabi-v7a -L/bak/android/ndk/sources/crystax/libs/armeabi-v7a

posix {
    MOC_DIR = /tmp
    OBJECTS_DIR = /tmp
}
win32 {
    MOC_DIR = c:/tmp
    OBJECTS_DIR = c:/tmp
}

#SOURCES += src/ui/main.cpp

include(src/ui/ui.pri)
#include(src/sockc/sockc.pri)

DISTFILES += \
    build/AndroidManifest.xml \
    build/gradle/wrapper/gradle-wrapper.jar \
    build/gradlew \
    build/res/values/libs.xml \
    build/build.gradle \
    build/gradle/wrapper/gradle-wrapper.properties \
    build/gradlew.bat

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/build
