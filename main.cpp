
#include <QtQml/QQmlContext>
#include <QtGui/QGuiApplication>
#include <QQmlApplicationEngine>

#include "appmodel.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    const char *uri = "org.p2plab.openvaccine";
    qmlRegisterType<AppModel>(uri, 1, 0, "AppModel");

    //! [0]
//    qRegisterMetaType<ScanData>();
    //! [1]

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:///main.qml")));

    return app.exec();
}


