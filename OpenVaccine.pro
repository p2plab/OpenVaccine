TEMPLATE = app

CONFIG += c++11

QT += core network qml quick svg xml concurrent

android {
    QT += androidextras

    OTHER_FILES += \
        android/AndroidManifest.xml

    ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
    ANDROID_EXTRA_LIBS += $$PWD/android/libs/libcrypto.so
    ANDROID_EXTRA_LIBS += $$PWD/android/libs/libssl.so

    DISTFILES += \
        android/src/org/p2plab/openvaccine/SendMail.java

    INCLUDEPATH +=$$PWD/ssl/include
    LIBS += -L$$PWD/ssl/armeabi-v7a/lib/ -lssl -lcrypto

}

INCLUDEPATH +=$$PWD/quazip

SOURCES += main.cpp \
    appmodel.cpp \
    sha1.cpp \
    quazip/qioapi.cpp \
    quazip/JlCompress.cpp \
    quazip/quaadler32.cpp \
    quazip/quacrc32.cpp \
    quazip/quagzipfile.cpp \
    quazip/quaziodevice.cpp \
    quazip/quazip.cpp \
    quazip/quazipdir.cpp \
    quazip/quazipfile.cpp \
    quazip/quazipfileinfo.cpp \
    quazip/quazipnewinfo.cpp \
    quazip/unzip.c \
    quazip/zip.c

RESOURCES += qml.qrc

OTHER_FILES += $PWD/translations/README

# var, prepend, append
defineReplace(prependAll) {
    for(a,$$1):result += $$2$${a}$$3
    return($$result)
}
# Supported languages
LANGUAGES = ko #bn da de es fi fr hi hu it nb nl pl ro ru zh

# Available translations
TRANSLATIONS = $$prependAll(LANGUAGES, $$PWD/translations/OpenVaccine_, .ts)

# Used to embed the qm files in resources
TRANSLATIONS_FILES =

# run LRELEASE to generate the qm files
qtPrepareTool(LRELEASE, lrelease)
for(tsfile, TRANSLATIONS) {
    qmfile = $$shadowed($$tsfile)
    qmfile ~= s,\\.ts$,.qm,
    qmdir = $$dirname(qmfile)
    !exists($$qmdir) {
        mkpath($$qmdir)|error("Aborting.")
    }
    command = $$LRELEASE -removeidentical $$tsfile -qm  $$qmfile
    system($$command)|error("Failed to run: $$command")
    TRANSLATIONS_FILES += $$qmfile
}

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH = components

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    appmodel.h \
    sha1.h \
    bloom_filter.h \
    ssl/include/openssl/pkcs7.h \
    ssl/include/openssl/evp.h \
    ssl/include/openssl/crypto.h \
    quazip/quazip.h \
    quazip/quazipfile.h \
    quazip/quagzipfile.h \
    quazip/quaziodevice.h \


