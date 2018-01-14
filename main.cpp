#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
//#include <QtWebEngine>
#include "core.h"
//#include "test.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);


    //        QStringList paths = QGuiApplication::libraryPaths();
    //        qDebug()<<"path"<<paths;
    //        paths.append(".");
    //        paths.append("platforms");
    //        QGuiApplication::setLibraryPaths(paths);
    //        app.addLibraryPath(app.applicationDirPath()+"/qml");
    //        app.addLibraryPath(app.applicationDirPath()+"/plugins");
    //        app.addLibraryPath(app.applicationDirPath()+"/plugins/platforms");
    //        app.addLibraryPath(app.applicationDirPath()+"/plugins/imageformats");
    //        qDebug()<<"dirPath"<<app.applicationDirPath();
    //        qDebug()<<"lobPaht"<<app.libraryPaths();


    QStringList paths = QCoreApplication::libraryPaths();
    paths.append(app.applicationDirPath()+"/imageformats");
    paths.append(app.applicationDirPath()+"/platforms");
    paths.append(app.applicationDirPath()+"/qml");
    QCoreApplication::setLibraryPaths(paths);

    //    qDebug()<<paths;


    QQmlApplicationEngine engine;

    Core core;

    //    QtWebEngine::initialize();
    //    QDesktopServices::openUrl(QUrl("http://www.cyberforum.ru/beta-testing/thread617608.html"));


    QQmlContext *ctxt = engine.rootContext();
    ctxt->setContextProperty("_core", &core);
    ctxt->setContextProperty("applicationDirPath", QGuiApplication::applicationDirPath());


    QString str;
    str = "file:///"+QGuiApplication::applicationDirPath();
    str += "/flat/main.qml";

    engine.load(QUrl((str)));
    //    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    //    engine.load(("D:/_svn/siborion/pangaea_skins/flat/main.qml"));
    //      engine.load(("C:/flat/main.qml"));

    return app.exec();
}
