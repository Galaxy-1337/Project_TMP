#include <QCoreApplication>
#include "minesweeper_server.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    MineSweeperServer server(5555);
    return app.exec();
}
