

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtQml/QQmlEngine>
#include <QtQml/QQmlContext>

//#include <QtGui/QFont>
//#include <QtGui/QFontDatabase>
#include <QtGui/QGuiApplication>
#include <QtQuick/QQuickView>
#include <QtQuick/QQuickItem>
#include <QLoggingCategory>

#include <appmodel.h>

int main(int argc, char *argv[])
{
    QLoggingCategory::setFilterRules("wapp.*.debug=false");
    QGuiApplication app(argc, argv);
    qmlRegisterType<ScanData>("OpenVaccine", 1, 0, "ScanData");
    qmlRegisterType<AppModel>("OpenVaccine", 1, 0, "AppModel");

    //! [0]
        qRegisterMetaType<ScanData>();
    //! [1]

#if 0
    QQuickView view;
    view.setSource(QUrl(QStringLiteral("qrc:///main.qml")));
    view.setResizeMode(QQuickView::SizeRootObjectToView);

    QObject::connect(view.engine(), SIGNAL(quit()), qApp, SLOT(quit()));
    view.setGeometry(QRect(100, 100, 360, 640));
    view.show();
#else
    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:///main.qml")));
#endif
    return app.exec();
}


