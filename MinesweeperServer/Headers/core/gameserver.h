#ifndef GAMESERVER_H
#define GAMESERVER_H

#include <QTcpServer>
#include <QObject>
#include <QMap>

class QTcpSocket;
class AuthHandler;
class GameRoomManager;

class GameServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit GameServer(QObject *parent = nullptr);
    void startServer(quint16 port);
    void sendJsonMessage(QTcpSocket* socket, const QJsonObject& message);

protected:
    void incomingConnection(qintptr handle) override;

private slots:
    void onClientDisconnected(QTcpSocket *socket);

private:
    QMap<QTcpSocket*, AuthHandler*> m_clients;
    GameRoomManager *m_roomManager;

    void handleAuthenticatedClient(QTcpSocket *socket, AuthHandler *authHandler);
};

#endif // GAMESERVER_H
