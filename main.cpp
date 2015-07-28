#include <QGuiApplication>
#include <QQmlApplicationEngine>


#include <QtGui/QGuiApplication>
#include <QtQuick/QQuickView>
#include <QtQml/QQmlEngine>
#include <QtQml/QQmlContext>
#include <QtQuick/QQuickItem>
#include <QLoggingCategory>

#include <appmodel.h>

int main(int argc, char *argv[])
{

#if 0
    QLoggingCategory::setFilterRules("wapp.*.debug=false");
    QGuiApplication app(argc, argv);

    qmlRegisterType<VirusData>("OpenVaccine", 1, 0, "ScanData");
    qmlRegisterType<AppModel>("OpenVaccine", 1, 0, "AppModel");

//! [0]
    qRegisterMetaType<VirusData>();
//! [1]
    const QString mainQmlApp = QStringLiteral("qrc:///main.qml");
    QQuickView view;
    view.setSource(QUrl(mainQmlApp));
    view.setResizeMode(QQuickView::SizeRootObjectToView);

    QObject::connect(view.engine(), SIGNAL(quit()), qApp, SLOT(quit()));
    view.setGeometry(QRect(100, 100, 360, 640));
    view.show();
    return app.exec();
#else
    QGuiApplication app(argc, argv);

    qmlRegisterType<ScanData>("OpenVaccine", 1, 0, "ScanData");
    qmlRegisterType<AppModel>("OpenVaccine", 1, 0, "AppModel");

//! [0]
    qRegisterMetaType<ScanData>();

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:///main.qml")));
    return app.exec();
#endif
}


