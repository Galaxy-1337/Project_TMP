#include "Headers/core/clientengine.h"
#include "Headers/ui/mainwindow.h"
#include "Headers/network/clientnetwork.h"

ClientEngine::ClientEngine(QObject *parent) : QObject(parent) {
    m_mainWindow = new MainWindow();
    m_network = new ClientNetwork();
}

void ClientEngine::start() {
    m_mainWindow->show();
}
