

VPATH += src/ui src/sockc
INCLUDEPATH += . src/ui src/sockc
DEPENDPATH += . src/ui src/sockc

unix:OBJECTS_DIR = /tmp
win32:OBJECTS_DIR = c:/tmp

include(src/ui/ui.pri)
