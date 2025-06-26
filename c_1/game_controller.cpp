#include "game_controller.h"
#include <QDebug>

game_controller::game_controller(game_model *model, QObject *parent)
    : QObject(parent), model(model) {
    socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::readyRead, this, &game_controller::readServerData);
    connect(socket, &QTcpSocket::disconnected, this, &game_controller::handleDisconnected);
}

void game_controller::connectToServer(const QString& name, const QString& sessionCode) {
    playerName = name;
    socket->connectToHost("127.0.0.1", 5555);
    if (socket->waitForConnected(5000)) {
        QString joinCommand = QString("JOIN %1 %2").arg(name, sessionCode);
        socket->write((joinCommand + "\n").toUtf8());
        socket->flush();
        qDebug() << "Sent JOIN command:" << joinCommand;
    } else {
        qDebug() << "Failed to connect to server";
    }
}

void game_controller::processCellClick(int x, int y, bool rightClick) {
    if (!model->isOpened(x, y)) {
        QString command = rightClick ? QString("FLAG %1 %2").arg(x).arg(y)
                                    : QString("OPEN %1 %2").arg(x).arg(y);
        socket->write((command + "\n").toUtf8());
        socket->flush();
        qDebug() << "Sent command:" << command;
    }
}

void game_controller::readServerData() {
    while (socket->canReadLine()) {
        QString data = QString::fromUtf8(socket->readLine()).trimmed();
        qDebug() << "Received from server:" << data;

        QStringList parts = data.split(" ");
        if (parts.isEmpty()) return;

        QString action = parts[0];
        if (action == "JOIN_OK") {
            qDebug() << "Joined successfully";
        } else if (action == "JOIN_ERROR") {
            qDebug() << "Join error:" << parts.mid(1).join(" ");
        } else if (action == "GAME_START" && parts.size() == 4) {
            bool ok1, ok2, ok3;
            int width = parts[1].toInt(&ok1);
            int height = parts[2].toInt(&ok2);
            parts[3].toInt(&ok3); // Игнорируем mines, чтобы убрать предупреждение
            if (ok1 && ok2 && ok3) {
                model->reset(width, height);
            }
        } else if (action == "UPDATE_CELL" && parts.size() == 4) {
            bool ok1, ok2, ok3;
            int x = parts[1].toInt(&ok1);
            int y = parts[2].toInt(&ok2);
            int value = parts[3].toInt(&ok3);
            if (ok1 && ok2 && ok3) {
                model->updateCell(x, y, value);
            }
        } else if (action == "FLAG_CELL" && parts.size() == 4) {
            bool ok1, ok2, ok3;
            int x = parts[1].toInt(&ok1);
            int y = parts[2].toInt(&ok2);
            bool flagged = parts[3].toInt(&ok3) == 1;
            if (ok1 && ok2 && ok3) {
                model->setFlag(x, y, flagged);
            }
        } else if (action == "GAME_OVER") {
            QString reason = parts.mid(1).join(" ");
            model->gameOver(reason);
        } else if (action == "GAME_WIN") {
            model->gameWon();
        }
    }
}

void game_controller::handleDisconnected() {
    qDebug() << "Disconnected from server";
    model->gameOver("DISCONNECTED");
}
