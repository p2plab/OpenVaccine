TEMPLATE = app

QT += core network positioning qml quick svg xml concurrent

SOURCES += main.cpp \
    appmodel.cpp \
    sha1.cpp

RESOURCES += qml.qrc

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
ANDROID_EXTRA_LIBS += $$PWD/android/libs/libcrypto.so
ANDROID_EXTRA_LIBS += $$PWD/android/libs/libssl.so

OTHER_FILES += \
    android/AndroidManifest.xml

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH = components

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    appmodel.h \
    sha1.h \
    bloom_filter.h
