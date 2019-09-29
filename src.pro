TARGET   = PangaeaCPPA
TEMPLATE = app

QT += qml quick svg
QT += serialport
#QT += texttospeech

CONFIG += c++11
CONFIG += no_batch

SOURCES += main.cpp \
    core.cpp \
    parser.cpp

RC_FILE     = resources.rc

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
    parser.h

OTHER_FILES += \
    ../flat/*.qml \
    ../flat/elements/*.qml \
    ../flat/moduls/*.qml


DESTDIR = $${BIN_PATH}/

win32 {
DESTDIR_WIN = $${DESTDIR}
DESTDIR_WIN ~= s,/,\\,g
QMAKE_POST_LINK += windeployqt.exe  release$${DESTDIR}$${TARGET}.exe  --release -qmldir=$${PWD}/release/flat/ --verbose 0 $$RETURN
}

macx {
QMAKE_POST_LINK += macdeployqt $${DESTDIR}$${TARGET}.app -qmldir=$${PWD}/ -dmg  $$RETURN
}




