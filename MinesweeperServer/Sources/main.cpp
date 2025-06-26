#include "Headers/core/gameserver.h"
#include <QCoreApplication>

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    GameServer server;
    server.startServer(12345);

    return a.exec();
}
