// Headers/core/clientinfo.h
#ifndef CLIENTINFO_H
#define CLIENTINFO_H

#include <QString>
#include <QTcpSocket>

struct ClientInfo {
    QString username;
    QString authToken;
    int rating;
    QTcpSocket* socket;

    // Для сравнения/хранения
    bool operator==(const ClientInfo& other) const {
        return username == other.username && authToken == other.authToken;
    }
};

#endif // CLIENTINFO_H
