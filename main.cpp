#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "core.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setOrganizationName("AMT");
    app.setApplicationName("Pangaea");

    FileModel fileModel;
    fileModel.addMFile(MFile("Wolf", "Medium"));
    fileModel.addMFile(MFile("Polar bear", "Large"));
    fileModel.addMFile(MFile("Quoll", "Small"));

    QQmlApplicationEngine engine;

    Core core;

//    qmlRegisterType<FileModel>("todt",1,2,"efv");

    QQmlContext *ctxt = engine.rootContext();

    ctxt->setContextProperty("fileModel", &fileModel);
    ctxt->setContextProperty("_core", &core);
    ctxt->setContextProperty("applicationDirPath", QGuiApplication::applicationDirPath());
    QString str;
    str = "file:///"+QGuiApplication::applicationDirPath();
    str += "/flat/main.qml";
    engine.load(QUrl((str)));
    return app.exec();
}
