// FileName: /gameroommanager.cpp
#include "Headers/core/gameroommanager.h"
#include "Headers/core/gameroom.h"
#include "Headers/core/clientinfo.h"
#include "Headers/core/authhandler.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QTcpSocket>
#include <QDebug>
#include <QJsonDocument>
#include <QUuid>

GameRoomManager::GameRoomManager(QObject *parent)
    : QObject(parent)
{
    qDebug() << "GameRoomManager initialized";
}

void GameRoomManager::addClient(QTcpSocket *socket, const ClientInfo &info)
{
    m_clientInfo[socket] = info;
    m_availableClients.insert(socket);
    qDebug() << "GameRoomManager: Client" << info.username << "added to available clients.";

    // Уведомляем нового клиента о списке комнат
    broadcastRoomsList();

    // Подключаем readyRead только для обработки сообщений лобби/комнат
    // Сообщения аутентификации уже обработаны AuthHandler
    connect(socket, &QTcpSocket::readyRead, this, [this, socket]() {
        handleClientReadyRead(socket);
    });
}

void GameRoomManager::handleClientReadyRead(QTcpSocket *socket) {
    while (socket->bytesAvailable() > 0) {
        QByteArray data = socket->readAll();
        qDebug() << "GameRoomManager: Raw data received from client:" << data;

        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(data, &error);

        if (error.error == QJsonParseError::NoError) {
            const QJsonObject msg = doc.object();
            const int msgType = msg[JsonFields::TYPE].toInt();

            qDebug() << "GameRoomManager: Message type:" << msgType;

            // Проверяем, находится ли клиент в комнате
            if (m_clientToRoomMap.contains(socket)) {
                QString roomId = m_clientToRoomMap[socket];
                if (m_rooms.contains(roomId)) {
                    // Перенаправляем сообщение в соответствующую игровую комнату
                    m_rooms[roomId]->handleClientMessage(socket, msg);
                    return; // Сообщение обработано комнатой
                }
            }

            // Обработка сообщений, не связанных с игрой в комнате
            switch (msgType) {
            case MessageType::CREATE_LOBBY: {
                QString roomId = createRoom(socket);
                QJsonObject response;
                response[JsonFields::TYPE] = MessageType::LOBBY_CREATED;
                response[JsonFields::ROOM_ID] = roomId;
                socket->write(QJsonDocument(response).toJson(QJsonDocument::Compact) + "\n");
                qDebug() << "GameRoomManager: LOBBY_CREATED sent for room:" << roomId;
                break;
            }
            case MessageType::JOIN_LOBBY: {
                bool success = joinRoom(socket, msg[JsonFields::ROOM_ID].toString());
                QJsonObject response;
                response[JsonFields::TYPE] = MessageType::JOIN_RESULT;
                response["success"] = success;
                socket->write(QJsonDocument(response).toJson(QJsonDocument::Compact) + "\n");
                qDebug() << "GameRoomManager: JOIN_RESULT sent. Success:" << success;
                break;
            }
            case MessageType::GET_LOBBIES: {
                qDebug() << "GameRoomManager: Received GET_LOBBIES request from client:" << socket->peerAddress().toString();
                broadcastRoomsList(); // Отправка списка лобби обратно клиенту
                break;
            }
            default:
                qWarning() << "GameRoomManager: Unknown message type or message not for lobby:" << msgType;
                break;
            }
        } else {
            qWarning() << "GameRoomManager: JSON parse error:" << error.errorString();
        }
    }
}

void GameRoomManager::clientDisconnected(QTcpSocket *socket)
{
    if (m_clientToRoomMap.contains(socket)) {
        QString roomId = m_clientToRoomMap[socket];
        if (m_rooms.contains(roomId)) {
            m_rooms[roomId]->playerDisconnected(socket);
            // Если комната закрылась после отключения игрока, она будет удалена через сигнал closed
        }
        m_clientToRoomMap.remove(socket);
    }

    m_clientInfo.remove(socket);
    m_availableClients.remove(socket);
    qDebug() << "GameRoomManager: Client disconnected. Updating lobby list.";
    broadcastRoomsList();
}

QString GameRoomManager::createRoom(QTcpSocket *ownerSocket)
{
    QString roomId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    // Передаем ownerSocket и его ClientInfo в конструктор GameRoom
    GameRoom *room = new GameRoom(ownerSocket, m_clientInfo[ownerSocket], this);

    m_rooms[roomId] = room;
    m_clientToRoomMap[ownerSocket] = roomId;
    m_availableClients.remove(ownerSocket); // Владелец комнаты больше не "доступен" в лобби
    qDebug() << "GameRoomManager: Room" << roomId << "created by" << m_clientInfo[ownerSocket].username;

    connect(room, &GameRoom::closed, this, [this, roomId, room]() {
        qDebug() << "GameRoomManager: Room" << roomId << "closed. Deleting GameRoom object.";
        // Перемещаем всех игроков из этой комнаты обратно в m_availableClients
        for (auto it = m_clientToRoomMap.begin(); it != m_clientToRoomMap.end(); ) {
            if (it.value() == roomId) {
                m_availableClients.insert(it.key());
                it = m_clientToRoomMap.erase(it); // Удаляем запись и переходим к следующей
            } else {
                ++it;
            }
        }
        m_rooms.remove(roomId);
        room->deleteLater(); // Удаляем объект GameRoom
        qDebug() << "GameRoomManager: Lobby list updated after room closure.";
        broadcastRoomsList();
    });

    broadcastRoomsList();
    return roomId;
}

bool GameRoomManager::joinRoom(QTcpSocket *playerSocket, const QString &roomId)
{
    if (!m_rooms.contains(roomId) || !m_availableClients.contains(playerSocket)) {
        qWarning() << "GameRoomManager: Failed to join room:" << roomId << ". Room does not exist or client is already in room/unavailable.";
        return false;
    }

    GameRoom *room = m_rooms[roomId];
    if (room->addPlayer(playerSocket, m_clientInfo[playerSocket])) {
        m_clientToRoomMap[playerSocket] = roomId;
        m_availableClients.remove(playerSocket); // Игрок больше не "доступен" в лобби
        qDebug() << "GameRoomManager: Client" << m_clientInfo[playerSocket].username << "joined room" << roomId;
        broadcastRoomsList();
        return true;
    }
    qWarning() << "GameRoomManager: Failed to join room:" << roomId << ". Room is full.";
    return false;
}

void GameRoomManager::broadcastRoomsList()
{
    QJsonArray roomsArray;
    for (auto it = m_rooms.begin(); it != m_rooms.end(); ++it) {
        roomsArray.append(serializeRoom(it.value()));
    }
    QJsonObject message;
    message[JsonFields::TYPE] = MessageType::LOBBY_LIST;
    message[JsonFields::LOBBY_LIST] = roomsArray;
    QByteArray data = QJsonDocument(message).toJson(QJsonDocument::Compact) + "\n";
    qDebug() << "GameRoomManager: Broadcasting lobby list:" << data;
    // Итерируемся по всем клиентам, которые есть в m_clientInfo (все подключенные)
    // Вместо m_availableClients
    for (QTcpSocket *client : m_clientInfo.keys()) { // <-- ИЗМЕНЕНИЕ ЗДЕСЬ
        if (client && client->isOpen() && client->isWritable()) {
            client->write(data);
        } else {
            qWarning() << "GameRoomManager: Could not send lobby list to client (socket closed or unavailable):" << (client ? client->peerAddress().toString() : "nullptr");
        }
    }
}

QJsonObject GameRoomManager::serializeRoom(GameRoom *room) const
{
    return {
        {JsonFields::ROOM_ID, m_rooms.key(room)},
        {JsonFields::OWNER, room->ownerInfo().username},
        {"players_count", room->playersCount()},
        {"max_players", 2} // Для сапёра всегда 2 игрока
    };
}
