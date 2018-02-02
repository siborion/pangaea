#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "core.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    Core core;
    QQmlContext *ctxt = engine.rootContext();
    ctxt->setContextProperty("_core", &core);
    ctxt->setContextProperty("applicationDirPath", QGuiApplication::applicationDirPath());
    QString str;
    str = "file:///"+QGuiApplication::applicationDirPath();
    str += "/flat/main.qml";
    engine.load(QUrl((str)));
    return app.exec();
}
