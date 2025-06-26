#ifndef MINESWEEPERSERVER_H
#define MINESWEEPERSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include "game_model.h"
#include "db_manager.h"


class MineSweeperServer : public QObject {
    Q_OBJECT
public:
    explicit MineSweeperServer(quint16 port, QObject *parent = nullptr);

private slots:
    void handleNewConnection();
    void readClientData();
    void handleClientDisconnection();
    void onGameStarted();
    void onCellUpdated(int x, int y, int value);
    void onFlagToggled(int x, int y, bool flagged);
    void onGameOver(bool mineHit);
    void onGameWon();
    void endGame(const QString& result);
private:
    void broadcast(const QString& message);
    bool checkSessionCode(const QString& code);

    QTcpServer* server;
    QList<QTcpSocket*> clients;
    game_model* model;
    QString sessionCode; // Хранит код сессии
    DBManager* dbManager;
    QDateTime gameStartTime;
};

#endif // MINESWEEPERSERVER_H
