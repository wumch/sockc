
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QtQml>
#include "Interact.hpp"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QTranslator *translator = new QTranslator;
    if (translator->load(":/i18n/" + QLocale::system().name()))
    {
        app.installTranslator(translator);
    }

//    qmlRegisterType<Interact>("Interact", 1, 0, "Interact");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return app.exec();
}

