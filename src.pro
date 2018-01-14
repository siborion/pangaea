TARGET   = PangaeaCPPA
TEMPLATE = app

QT += qml quick svg
QT += serialport

#QT += multimedia

#DESTDIR = pang

CONFIG += c++11
CONFIG += no_batch

SOURCES += main.cpp \
    core.cpp \
    parser.cpp

# RESOURCES += qml.qrc

RC_FILE     = resources.rc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    ../pangaea_skins/flat/main.qml \
    ../pangaea_skins/flat/moduls/Common.qml \
    ../pangaea_skins/flat/elements/Triangle.qml \
    ../pangaea_skins/flat/moduls/Gt.qml \
    ../pangaea_skins/flat/elements/Dial.qml \
    ../pangaea_skins/flat/moduls/Moduls.qml \
    ../pangaea_skins/flat/moduls/Cm.qml \
    ../pangaea_skins/flat/moduls/Pr.qml \
    ../pangaea_skins/flat/moduls/Ir.qml \
    ../pangaea_skins/flat/moduls/Pa.qml \
    ../pangaea_skins/flat/moduls/Hp.qml \
    ../pangaea_skins/flat/moduls/Vl.qml \
    ../pangaea_skins/flat/moduls/Er.qml \
    ../pangaea_skins/flat/moduls/Ps.qml \
    ../pangaea_skins/flat/moduls/Lp.qml \
    ../pangaea_skins/flat/elements/Pot.qml \
    ../pangaea_skins/flat/moduls/Eqs.qml \
    ../pangaea_skins/flat/elements/Eq.qml \
    ../pangaea_skins/flat/elements/Pot.qml \
    ../pangaea_skins/flat/moduls/Head.qml \
    ../pangaea_skins/flat/elements/TypePA.qml \
    ../pangaea_skins/flat/elements/CheckText.qml \
    ../pangaea_skins/flat/elements/Switch2.qml \
    ../pangaea_skins/flat/elements/Switch3.qml \
    ../pangaea_skins/flat/elements/SwitchIr.qml \
    ../pangaea_skins/flat/elements/SwitchRoom.qml \
    ../pangaea_skins/flat/elements/SwitchOutput.qml \
    ../pangaea_skins/flat/moduls/OutputType.qml \
    ../pangaea_skins/flat/elements/Switch3v.qml \
    ../pangaea_skins/flat/elements/SwitchPostPre.qml \
    ../pangaea_skins/flat/elements/MButton.qml \
    ../pangaea_skins/flat/moduls/SaveComp.qml \
    ../pangaea_skins/flat/moduls/CopyPast.qml \
    ../pangaea_skins/flat/elements/Presets.qml \
    ../pangaea_skins/flat/elements/Tumbler1.qml.txt \
    ../pangaea_skins/flat/elements/UpDown.qml \
    ../pangaea_skins/flat/moduls/EqsMap.qml \
    ../pangaea_skins/flat/elements/SwitchEqMap.qml \
    ../pangaea_skins/flat/moduls/Map.qml \
    ../pangaea_skins/flat/elements/MapRow.qml \
    ../pangaea_skins/flat/elements/MapHeadBank.qml \
    ../pangaea_skins/flat/elements/MapHeadPreset.qml \
    ../pangaea_skins/flat/MFileOpen.qml \
    ../pangaea_skins/flat/MDrivers.qml \
    ../pangaea_skins/flat/MBusy.qml \
    ../pangaea_skins/flat/elements/Material.qml \
    ../pangaea_skins/flat/elements/MaterialButton.qml \
    qml/default.qml \
    ../pangaea_skins/flat/moduls/Logo.qml



HEADERS += \
    core.h \
    struct.h \
    test.h \
    parser.h


