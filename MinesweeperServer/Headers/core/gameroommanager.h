#ifndef GAMEROOMMANAGER_H
#define GAMEROOMMANAGER_H

#include <QObject>
#include <QMap>
#include <QSet>
#include "Headers/network/protocol_defines.h"
#include "Headers/core/clientinfo.h"

class QTcpSocket;
class GameRoom;


class GameRoomManager : public QObject
{
    Q_OBJECT
public:
    explicit GameRoomManager(QObject *parent = nullptr);

    void addClient(QTcpSocket *socket, const ClientInfo &info);
    void clientDisconnected(QTcpSocket *socket);
    QString createRoom(QTcpSocket *ownerSocket);
    bool joinRoom(QTcpSocket *playerSocket, const QString &roomId);
    void handleClientReadyRead(QTcpSocket *socket);

signals:
    void roomsListUpdated(const QJsonArray &rooms);

private:
    void broadcastRoomsList();
    QJsonObject serializeRoom(GameRoom *room) const;

    QMap<QString, GameRoom*> m_rooms;
    QMap<QTcpSocket*, QString> m_clientToRoomMap;
    QMap<QTcpSocket*, ClientInfo> m_clientInfo;
    QSet<QTcpSocket*> m_availableClients;
};
#endif // GAMEROOMMANAGER_H
