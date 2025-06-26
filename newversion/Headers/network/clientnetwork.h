#ifndef CLIENTNETWORK_H
#define CLIENTNETWORK_H

#include <QObject>
#include <QTcpSocket>
#include <QJsonObject>
#include "Headers/network/protocol_defines.h"

class ClientNetwork : public QObject
{
    Q_OBJECT
public:
    explicit ClientNetwork(QObject *parent = nullptr);
    bool connectToServer(const QString &host, quint16 port);
    void sendMessage(const QJsonObject &message);

signals:
    void messageReceived(const QJsonObject &message);
    void connectionStateChanged(bool connected);

private slots:
    void onReadyRead();
    void onStateChanged(QAbstractSocket::SocketState state);

private:
    QTcpSocket *m_socket;
    QByteArray m_readBuffer; // Буфер для неполных сообщений
};

#endif // CLIENTNETWORK_H
