#ifndef CLIENTENGINE_H
#define CLIENTENGINE_H

#include <QObject>
#include "Headers/network/clientnetwork.h"
#include "Headers/ui/mainwindow.h"

class ClientEngine : public QObject {
    Q_OBJECT
public:
    explicit ClientEngine(QObject *parent = nullptr);
    void start();

private:
    MainWindow *m_mainWindow;
    ClientNetwork *m_network;
};

#endif // CLIENTENGINE_H
