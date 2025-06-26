// FileName: /gameroom.cpp
#include "Headers/core/gameroom.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QUuid>
#include <QDebug>

GameRoom::GameRoom(QTcpSocket* owner, const ClientInfo& ownerInfo, QObject* parent)
    : QObject(parent), m_owner(owner), m_ownerInfo(ownerInfo) {
    m_id = QUuid::createUuid().toString();
    // Добавляем владельца комнаты как первого игрока
    m_players.insert(owner, {ownerInfo.username, GameLogic::Scout}); // Владелец всегда Разведчик
    // Отправляем информацию о роли владельцу
    QJsonObject roleMsg;
    roleMsg[JsonFields::TYPE] = MessageType::ROLE_ASSIGNED;
    roleMsg[JsonFields::ROLE] = JsonFields::ROLE_SCOUT;
    sendToPlayer(owner, roleMsg);
}

bool GameRoom::addPlayer(QTcpSocket* socket, const ClientInfo& playerInfo)
{
    if (m_players.size() >= 2) return false;

    // Если владелец уже добавлен, то второй игрок будет Диверсантом
    auto role = GameLogic::Saboteur;
    m_players.insert(socket, {playerInfo.username, role});

    // Отправляем информацию о роли игроку
    QJsonObject roleMsg;
    roleMsg[JsonFields::TYPE] = MessageType::ROLE_ASSIGNED;
    roleMsg[JsonFields::ROLE] = role == GameLogic::Scout ? JsonFields::ROLE_SCOUT : JsonFields::ROLE_SABOTEUR;
    sendToPlayer(socket, roleMsg);

    if (m_players.size() == 2) {
        startGame();
    }

    return true;
}

void GameRoom::startGame() {
    m_gameLogic = std::make_unique<GameLogic>();
    m_gameLogic->initializeGame(10, 10, 15);

    connect(m_gameLogic.get(), &GameLogic::gameFieldChanged,
            this, &GameRoom::broadcastGameState);
    connect(m_gameLogic.get(), &GameLogic::gameOver,
            this, &GameRoom::sendGameOver);
    connect(m_gameLogic.get(), &GameLogic::playerSwitched,
            this, [this](GameLogic::PlayerRole newPlayer){
                // Обновляем состояние игры, чтобы отправить текущий ход
                broadcastGameState();
            });
    connect(m_gameLogic.get(), &GameLogic::scoresChanged,
            this, [this](int scoutScore, int saboteurScore){
                // Обновляем состояние игры, чтобы отправить новые очки
                broadcastGameState();
            });


    // Отправляем начальное состояние игры
    QJsonObject startMsg;
    startMsg[JsonFields::TYPE] = MessageType::GAME_START;
    startMsg[JsonFields::ROWS] = m_gameLogic->rows();
    startMsg[JsonFields::COLS] = m_gameLogic->cols();
    // Добавляем роли игроков в сообщение GAME_START
    for (auto it = m_players.begin(); it != m_players.end(); ++it) {
        QJsonObject playerRoleInfo = startMsg;
        playerRoleInfo[JsonFields::ROLE] = it.value().second == GameLogic::Scout ? JsonFields::ROLE_SCOUT : JsonFields::ROLE_SABOTEUR;
        sendToPlayer(it.key(), playerRoleInfo);
    }

    broadcastGameState();
}

void GameRoom::handleClientMessage(QTcpSocket* sender, const QJsonObject& msg) {
    if (!m_players.contains(sender) || !m_gameLogic) return;

    int type = msg[JsonFields::TYPE].toInt();
    auto playerRole = m_players[sender].second;

    if (type == MessageType::GAME_MOVE) {
        int row = msg[JsonFields::ROW].toInt();
        int col = msg[JsonFields::COL].toInt();

        bool success = false;
        if (playerRole == m_gameLogic->currentPlayer()) { // Проверяем, что это ход текущего игрока
            if (playerRole == GameLogic::Scout) {
                success = m_gameLogic->openCell(row, col, GameLogic::Scout);
            } else { // Saboteur
                success = m_gameLogic->toggleCellMark(row, col);
            }
        } else {
            qWarning() << "Неправильный ход: не очередь игрока" << m_players[sender].first;
        }

        if (success) {
            m_gameLogic->switchPlayerTurn();
        }
    } else if (type == MessageType::SURRENDER) {
        qDebug() << "Игрок" << m_players[sender].first << "сдался.";
        // Определяем победителя при сдаче
        QString winnerRole = (playerRole == GameLogic::Scout) ? JsonFields::ROLE_SABOTEUR : JsonFields::ROLE_SCOUT;
        QJsonObject gameOverMsg;
        gameOverMsg[JsonFields::TYPE] = MessageType::GAME_OVER;
        gameOverMsg[JsonFields::WINNER] = winnerRole;
        broadcastMessage(gameOverMsg);
        emit closed(); // Закрываем комнату после сдачи
    }
}

void GameRoom::broadcastGameState() {
    QJsonObject state;
    state[JsonFields::TYPE] = MessageType::GAME_UPDATE;
    state[JsonFields::SCOUT_SCORE] = m_gameLogic->scoutScore();
    state[JsonFields::SABOTEUR_SCORE] = m_gameLogic->saboteurScore();
    state[JsonFields::CURRENT_TURN] = m_gameLogic->currentPlayer() == GameLogic::Scout ?
                                          JsonFields::ROLE_SCOUT : JsonFields::ROLE_SABOTEUR;

    QJsonArray field;
    const auto& gameField = m_gameLogic->gameField();
    for (int i = 0; i < m_gameLogic->rows(); ++i) {
        for (int j = 0; j < m_gameLogic->cols(); ++j) {
            QJsonObject cellObj;
            cellObj[JsonFields::ROW] = i;
            cellObj[JsonFields::COL] = j;
            cellObj[JsonFields::STATE] = gameField[i][j].state;
            cellObj[JsonFields::MINES_AROUND] = gameField[i][j].minesAround;
            cellObj[JsonFields::HAS_MINE] = gameField[i][j].hasMine; // Отправляем информацию о мине для отладки/отображения
            field.append(cellObj);
        }
    }
    state[JsonFields::FIELD] = field;

    broadcastMessage(state);
}

void GameRoom::sendGameOver(bool won) {
    QJsonObject msg;
    msg[JsonFields::TYPE] = MessageType::GAME_OVER;
    // Определяем победителя на основе счета или роли, если игра выиграна
    if (won) {
        msg[JsonFields::WINNER] = (m_gameLogic->scoutScore() > m_gameLogic->saboteurScore()) ?
                                      JsonFields::ROLE_SCOUT : JsonFields::ROLE_SABOTEUR;
    } else {
        // Если игра проиграна (например, разведчик наступил на мину), то победитель - диверсант
        msg[JsonFields::WINNER] = JsonFields::ROLE_SABOTEUR;
    }
    broadcastMessage(msg);
    emit closed(); // Закрываем комнату после завершения игры
}

void GameRoom::playerDisconnected(QTcpSocket* player) {
    if (m_players.contains(player)) {
        m_players.remove(player);
        qDebug() << "Игрок" << player->peerAddress().toString() << "отключился от комнаты" << m_id;

        // Если комната опустела — можно закрыть её
        if (m_players.isEmpty()) {
            emit closed();
        } else {
            // Если остался один игрок, он выигрывает
            QJsonObject gameOverMsg;
            gameOverMsg[JsonFields::TYPE] = MessageType::GAME_OVER;
            gameOverMsg[JsonFields::WINNER] = m_players.begin().value().second == GameLogic::Scout ? JsonFields::ROLE_SCOUT : JsonFields::ROLE_SABOTEUR;
            broadcastMessage(gameOverMsg);
            emit closed();
        }
    }
}

int GameRoom::playersCount() const {
    return m_players.size();
}

void GameRoom::sendToPlayer(QTcpSocket* socket, const QJsonObject& msg) {
    if (socket && socket->isOpen() && socket->isWritable()) {
        QByteArray data = QJsonDocument(msg).toJson(QJsonDocument::Compact) + "\n"; // Компактный формат для экономии трафика
        socket->write(data);
        qDebug() << "Отправлено сообщение игроку" << socket->peerAddress().toString() << ":" << msg;
        qDebug() << "GAME ROOM SENT TO PLAYER:" << data;
    } else {
        qWarning() << "Сокет закрыт или недоступен для записи:" << (socket ? socket->peerAddress().toString() : "nullptr");
    }
}

void GameRoom::broadcastMessage(const QJsonObject& msg) {
    QByteArray data = QJsonDocument(msg).toJson(QJsonDocument::Compact) + "\n";
    qDebug() << "Рассылаем сообщение:" << msg;
    qDebug() << "GAME ROOM BROADCAST:" << data;

    // Создаем список сокетов, чтобы избежать проблем с итератором при отключении
    QList<QTcpSocket*> socketsToSend;
    for (auto it = m_players.begin(); it != m_players.end(); ++it) {
        socketsToSend.append(it.key());
    }

    for (QTcpSocket* socket : socketsToSend) {
        if (socket && socket->isOpen() && socket->isWritable()) {
            socket->write(data);
        } else {
            qWarning() << "Сокет закрыт или недоступен для записи:" << (socket ? socket->peerAddress().toString() : "nullptr");
        }
    }
}

void GameRoom::removePlayer(QTcpSocket* socket) {
    // Эта функция не используется напрямую, но может быть полезна для явного удаления
    playerDisconnected(socket);
}

void GameRoom::closeRoom() {
    // Дополнительная логика закрытия комнаты, если требуется
    qDebug() << "Комната" << m_id << "закрывается.";
    // m_gameLogic будет автоматически удален благодаря unique_ptr
}
