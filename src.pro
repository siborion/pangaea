TARGET   = PangaeaCPPA
TEMPLATE = app

QT += qml quick svg
QT += serialport

CONFIG += c++11
CONFIG += no_batch

SOURCES += main.cpp \
    core.cpp \
    parser.cpp \
    resample.cpp

RC_FILE     = resources.rc

LIBS += -lsoxr  -L/usr/local/lib

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \

HEADERS += \
    core.h \
    struct.h \
    parser.h \
    resample.h


