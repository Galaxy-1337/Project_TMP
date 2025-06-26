#include "Headers/core/gameserver.h"
#include "Headers/core/authhandler.h"
#include "Headers/core/gameroommanager.h"
#include "Headers/core/clientinfo.h"
#include <QDebug>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

GameServer::GameServer(QObject *parent)
    : QTcpServer(parent), m_roomManager(new GameRoomManager(this))
{
    qDebug() << "GameServer initialized";
}

void GameServer::sendJsonMessage(QTcpSocket* socket, const QJsonObject& message)
{
    QByteArray data = QJsonDocument(message).toJson(QJsonDocument::Compact) + "\n";
    socket->write(data);
    socket->flush();
    qDebug() << "SERVER SENT:" << data;
}

void GameServer::startServer(quint16 port)
{
    if (!listen(QHostAddress::Any, port)) {
        qCritical() << "Failed to start server on port" << port;
    } else {
        qInfo() << "Server started on port" << port;
    }
}

void GameServer::incomingConnection(qintptr handle)
{
    QTcpSocket *clientSocket = new QTcpSocket(this);
    clientSocket->setSocketDescriptor(handle);
    qDebug() << "New client connected:" << clientSocket->peerAddress();

    AuthHandler *authHandler = new AuthHandler(clientSocket, this);
    m_clients.insert(clientSocket, authHandler);

    // Подключаем readyRead к AuthHandler для обработки первого сообщения (AUTH_REQUEST)
    connect(clientSocket, &QTcpSocket::readyRead, this, [this, clientSocket, authHandler]() {
        // Читаем только одно сообщение за раз, чтобы избежать смешивания
        // В реальном приложении нужна более сложная буферизация
        QByteArray data = clientSocket->readAll();
        qDebug() << "Raw data received from client (Auth phase):" << data;

        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(data, &error);

        if (error.error == QJsonParseError::NoError) {
            QJsonObject msg = doc.object();
            if (msg[JsonFields::TYPE].toInt() == MessageType::AUTH_REQUEST) {
                authHandler->handleAuthRequest(msg);
            } else {
                qWarning() << "GameServer: Received non-AUTH_REQUEST message during auth phase:" << msg;
                // Отправляем ошибку или закрываем соединение, если это не AUTH_REQUEST
            }
        } else {
            qWarning() << "GameServer: JSON parse error during auth phase:" << error.errorString() << "in data:" << data;
        }
    });

    connect(clientSocket, &QTcpSocket::disconnected, this, [this, clientSocket]() {
        onClientDisconnected(clientSocket);
    });

    connect(authHandler, &AuthHandler::authenticationSuccess, this, [this, clientSocket, authHandler]() {
        handleAuthenticatedClient(clientSocket, authHandler);
    });

    connect(authHandler, &AuthHandler::authenticationFailed, this, [this, clientSocket](const QString& reason) {
        QJsonObject authResponse;
        authResponse[JsonFields::TYPE] = MessageType::AUTH_RESPONSE;
        authResponse["success"] = false;
        authResponse[JsonFields::MESSAGE] = reason;
        sendJsonMessage(clientSocket, authResponse);
        qDebug() << "GameServer: Authentication failed for client" << clientSocket->peerAddress() << ". Reason:" << reason;
        // Можно закрыть сокет здесь, если не хотим, чтобы неаутентифицированные клиенты оставались подключенными
        // clientSocket->disconnectFromHost();
    });
}

void GameServer::handleAuthenticatedClient(QTcpSocket *socket, AuthHandler *authHandler)
{
    // Отключаем readyRead от AuthHandler, чтобы GameRoomManager мог его обрабатывать
    // Это важно, чтобы избежать двойной обработки сообщений
    disconnect(socket, &QTcpSocket::readyRead, nullptr, nullptr);
    qDebug() << "GameServer: Client" << authHandler->clientInfo().username << "authenticated. Disconnecting old readyRead.";


    // Отправляем сообщение об успешной аутентификации
    QJsonObject authResponse;
    authResponse[JsonFields::TYPE] = MessageType::AUTH_RESPONSE;
    authResponse["success"] = true;
    authResponse[JsonFields::USERNAME] = authHandler->clientInfo().username;
    authResponse[JsonFields::AUTH_TOKEN] = authHandler->clientInfo().authToken; // Используйте JsonFields::AUTH_TOKEN
    sendJsonMessage(socket, authResponse);
    qDebug() << "GameServer: Sent AUTH_RESPONSE to" << authHandler->clientInfo().username;

    // Добавляем клиента в RoomManager. Это вызовет broadcastRoomsList(), который отправит LOBBY_LIST.
    m_roomManager->addClient(socket, authHandler->clientInfo());
    qDebug() << "GameServer: Client" << authHandler->clientInfo().username << "added to RoomManager.";
}

void GameServer::onClientDisconnected(QTcpSocket *socket)
{
    if (m_clients.contains(socket)) {
        AuthHandler *handler = m_clients.take(socket);
        handler->deleteLater();
        m_roomManager->clientDisconnected(socket);
        qDebug() << "GameServer: Client" << socket->peerAddress() << "disconnected. AuthHandler deleted.";
    } else {
        qDebug() << "GameServer: Client" << socket->peerAddress() << "disconnected. No AuthHandler found.";
    }
    socket->deleteLater();
}
