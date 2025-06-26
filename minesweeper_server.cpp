#include "minesweeper_server.h"
#include <QDebug>

MineSweeperServer::MineSweeperServer(quint16 port, QObject *parent)
    : QObject(parent) {
    server = new QTcpServer(this);
    model = new game_model(10, 10, 10, this);
    dbManager = new DBManager(this);

    connect(server, &QTcpServer::newConnection, this, &MineSweeperServer::handleNewConnection);
    connect(model, &game_model::gameStarted, this, &MineSweeperServer::onGameStarted);
    connect(model, &game_model::cellUpdated, this, &MineSweeperServer::onCellUpdated);
    connect(model, &game_model::flagToggled, this, &MineSweeperServer::onFlagToggled);
    connect(model, &game_model::gameOver, this, &MineSweeperServer::onGameOver);
    connect(model, &game_model::gameWon, this, &MineSweeperServer::onGameWon);

    if (!server->listen(QHostAddress::Any, port)) {
        qDebug() << "Server could not start!";
    } else {
        qDebug() << "Server started on port" << port;
    }
}

void MineSweeperServer::handleNewConnection() {
    QTcpSocket* client = server->nextPendingConnection();
    clients.append(client);
    connect(client, &QTcpSocket::readyRead, this, &MineSweeperServer::readClientData);
    connect(client, &QTcpSocket::disconnected, this, &MineSweeperServer::handleClientDisconnection);

    qDebug() << "New client connected. Total clients:" << clients.size();
}

void MineSweeperServer::readClientData() {
    QTcpSocket* client = qobject_cast<QTcpSocket*>(sender());
    if (!client) return;

    QByteArray data = client->readAll();
    QString command = QString::fromUtf8(data).trimmed();
    qDebug() << "Received command:" << command;

    QStringList parts = command.split(" ");
    if (parts.isEmpty()) return;

    QString action = parts[0];
    if (action == "JOIN" && parts.size() == 3) {
        QString name = parts[1];
        QString sessionCode = parts[2];
        if (checkSessionCode(sessionCode)) {
            client->write("JOIN_OK\n");
            client->flush();
            qDebug() << "Client" << name << "joined with session code" << sessionCode;
            if (clients.size() == 2) {
                model->startNewGame();
            }
        } else {
            client->write("JOIN_ERROR Invalid session code\n");
            client->flush();
            client->disconnectFromHost();
            clients.removeOne(client);
        }
    } else if (action == "OPEN" && parts.size() == 3) {
        bool ok1, ok2;
        int x = parts[1].toInt(&ok1);
        int y = parts[2].toInt(&ok2);
        if (ok1 && ok2) {
            model->openCell(x, y);
        }
    } else if (action == "FLAG" && parts.size() == 3) {
        bool ok1, ok2;
        int x = parts[1].toInt(&ok1);
        int y = parts[2].toInt(&ok2);
        if (ok1 && ok2) {
            model->toggleFlag(x, y);
        }
    }
}

bool MineSweeperServer::checkSessionCode(const QString& code) {
    if (clients.size() == 1) {
        sessionCode = code;
        return true;
    } else if (clients.size() == 2) {
        return code == sessionCode;
    }
    return false;
}

void MineSweeperServer::handleClientDisconnection() {
    QTcpSocket* client = qobject_cast<QTcpSocket*>(sender());
    if (!client) return;

    clients.removeOne(client);
    client->deleteLater();
    qDebug() << "Client disconnected. Total clients:" << clients.size();

    if (clients.size() < 2) {
        onGameOver(false);
    }
}

void MineSweeperServer::onGameStarted() {
    gameStartTime = QDateTime::currentDateTime();
    QString startMessage = QString("GAME_START %1 %2 %3")
                               .arg(model->getWidth())
                               .arg(model->getHeight())
                               .arg(model->getMinesCount());
    broadcast(startMessage);
    qDebug() << "Game started!";
}

void MineSweeperServer::onCellUpdated(int x, int y, int value) {
    QString update = QString("UPDATE_CELL %1 %2 %3").arg(x).arg(y).arg(value);
    broadcast(update);
}

void MineSweeperServer::onFlagToggled(int x, int y, bool flagged) {
    QString update = QString("FLAG_CELL %1 %2 %3").arg(x).arg(y).arg(flagged ? 1 : 0);
    broadcast(update);
}

void MineSweeperServer::onGameOver(bool mineHit) {
    QString reason = mineHit ? "MINE_HIT" : "PLAYER_DISCONNECTED";
    broadcast("GAME_OVER " + reason);
    qDebug() << "Game over: " << reason;
    endGame(reason);
}

void MineSweeperServer::onGameWon() {
    broadcast("GAME_WIN");
    qDebug() << "Game won!";
    endGame("WIN");
}

void MineSweeperServer::endGame(const QString& result) {
    QDateTime endTime = QDateTime::currentDateTime();
    dbManager->logGame(sessionCode, result, gameStartTime, endTime,
                       model->getWidth(), model->getHeight(), model->getMinesCount());

    for (QTcpSocket* client : clients) {
        client->disconnectFromHost();
    }
    clients.clear();

    model->startNewGame();
}

void MineSweeperServer::broadcast(const QString& message) {
    QByteArray data = (message + "\n").toUtf8();
    for (QTcpSocket* client : clients) {
        if (client->state() == QAbstractSocket::ConnectedState) {
            client->write(data);
            client->flush();
        }
    }
    qDebug() << "Broadcasted:" << message;
}
