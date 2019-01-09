TARGET   = PangaeaCPPA
TEMPLATE = app

QT += qml quick svg
QT += serialport

CONFIG += c++11
CONFIG += no_batch

SOURCES += main.cpp \
    core.cpp \
    parser.cpp \
    resample.cpp \
    wavinfo.cpp

RC_FILE     = resources.rc

#LIBS += -lsox  -L/usr/local/lib
#LIBS += -lsox  -LD:\_svn\_AMT\pangaea_git\lib
#win32:  LIBS += -lftd2xx  -LD:\_SVN\_American\siborion_american_scan



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
    resample.h \
    wavinfo.h


