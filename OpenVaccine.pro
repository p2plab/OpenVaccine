TEMPLATE = app

QT += core network positioning qml quick svg xml

SOURCES += main.cpp \
    appmodel.cpp

RESOURCES += qml.qrc

android: ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

OTHER_FILES += \
    android/AndroidManifest.xml

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH = components

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    appmodel.h
