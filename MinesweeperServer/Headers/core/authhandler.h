#ifndef AUTHHANDLER_H
#define AUTHHANDLER_H

#include <QObject>
#include <QTcpSocket>
#include "Headers/network/protocol_defines.h"
#include "clientinfo.h"


class AuthHandler : public QObject {
    Q_OBJECT

public:
    explicit AuthHandler(QTcpSocket *socket, QObject *parent = nullptr);
    const ClientInfo& clientInfo() const;

signals:
    void authenticationSuccess();
    void authenticationFailed(const QString& reason);

public slots:
    void handleAuthRequest(const QJsonObject &request);

private:
    QTcpSocket *m_socket;
    ClientInfo m_clientInfo;
    bool validateCredentials(const QString &login, const QString &passwordHash);
};

#endif // AUTHHANDLER_H
