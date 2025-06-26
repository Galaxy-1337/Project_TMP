// FileName: /authhandler.cpp
#include "Headers/core/authhandler.h"
#include "Headers/core/clientinfo.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QUuid>
#include <QDebug> // Добавлено для qDebug

AuthHandler::AuthHandler(QTcpSocket *socket, QObject *parent)
    : QObject(parent), m_socket(socket) {}

void AuthHandler::handleAuthRequest(const QJsonObject& message) {
    QString login = message["login"].toString();
    QString passwordHash = message["password_hash"].toString();

    qDebug() << "AuthHandler: Handling auth request for login:" << login;

    if (validateCredentials(login, passwordHash)) {
        m_clientInfo.username = login;
        m_clientInfo.authToken = QUuid::createUuid().toString();
        m_clientInfo.rating = 100;
        m_clientInfo.socket = m_socket;

        qDebug() << "AuthHandler: Authentication successful for" << login;
        emit authenticationSuccess();
    } else {
        qDebug() << "AuthHandler: Authentication failed for" << login;
        emit authenticationFailed("Invalid credentials");
    }
}

bool AuthHandler::validateCredentials(const QString &login, const QString &passwordHash) {
    // Заглушка - в День 4 подключим БД
    // Для простоты, используем "admin" и "admin" как логин/пароль
    return (login == "admin" && passwordHash == "admin");
    // Если бы использовался реальный хеш, это выглядело бы так:
    // return (login == "admin" && passwordHash == "5e884898da28047151d0e56f8dc6292773603d0d6aabbdd62a11ef721d1542d8");
}

const ClientInfo& AuthHandler::clientInfo() const {
    return m_clientInfo;
}
