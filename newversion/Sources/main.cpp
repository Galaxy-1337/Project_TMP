#include "Headers/core/clientengine.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    ClientEngine engine;
    engine.start();

    return a.exec();
}
