
#include "sockc/predef.hpp"
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include "sockc/Config.hpp"
#include "sockc/Portal.hpp"
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QtQml>
#include <iostream>
#include "Interact.hpp"

namespace csocks {

void initUi(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QTranslator *translator = new QTranslator;
    if (translator->load(":/i18n/" + QLocale::system().name()))
    {
        app.installTranslator(translator);
    }

    qmlRegisterType<Interact>("Interact", 1, 0, "Interact");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    app.exec();
}

void initSockc(int argc, char* argv[])
{
    csocks::Portal::initialize(argc, argv);
    csocks::Portal portal;
    portal.run();
}

}

int main(int argc, char* argv[])
{
    boost::thread uiThread(boost::bind(csocks::initUi, argc, argv));
    boost::thread sockcThread(boost::bind(csocks::initSockc, argc, argv));
    uiThread.join();
    sockcThread.join();
}
