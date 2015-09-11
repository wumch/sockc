TEMPLATE = app

QT += qml quick widgets

SOURCES += Interact.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

CONFIG += localize_deployment
TRANSLATIONS += ts/zh_CN.ts

# Default rules for deployment.
include(deployment.pri)

HEADERS += Interact.hpp
