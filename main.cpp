
#include <QtQml/QQmlContext>
#include <QtGui/QGuiApplication>
#include <QQmlApplicationEngine>

#include <QSslSocket>
#include <QtQuick>

#include "appmodel.h"

int main(int argc, char *argv[])
{

    QGuiApplication app(argc, argv);

    const char *uri = "org.p2plab.openvaccine";
    qmlRegisterType<AppModel>(uri, 1, 0, "AppModel");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    if(QSslSocket::supportsSsl()){
        qDebug() << "SSL supported!";
    }else{
        qDebug() << "SSL not supported!";
    }

    return app.exec();
}


