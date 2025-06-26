// gameroom.h

#ifndef GAMEROOM_H
#define GAMEROOM_H

#include "gamelogic.h"
#include "headers/network/protocol_defines.h"
#include "clientinfo.h"
#include <QTcpSocket>
#include <QMap>
#include <memory>
#include <QPair>

class GameRoom : public QObject
{
    Q_OBJECT
public:
    explicit GameRoom(QTcpSocket* owner, const ClientInfo& ownerInfo, QObject* parent = nullptr);

    QString id() const { return m_id; }
    bool addPlayer(QTcpSocket* socket, const ClientInfo& playerInfo);
    void removePlayer(QTcpSocket* socket);
    void handleClientMessage(QTcpSocket* sender, const QJsonObject& msg);
    void playerDisconnected(QTcpSocket* player);

    const ClientInfo& ownerInfo() const { return m_ownerInfo; }
    int playersCount() const;

signals:
    void closed();

private slots:
    void startGame();
    void broadcastGameState();
    void sendGameOver(bool winnerIsScout);

private:
    void sendToPlayer(QTcpSocket* socket, const QJsonObject& msg);
    void broadcastMessage(const QJsonObject& msg);
    void closeRoom();

    QString m_id;
    QTcpSocket* m_owner;
    ClientInfo m_ownerInfo;
    std::unique_ptr<GameLogic> m_gameLogic;
    QMap<QTcpSocket*, QPair<QString, GameLogic::PlayerRole>> m_players;
};

#endif // GAMEROOM_H
