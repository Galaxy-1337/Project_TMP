// FileName: /lobbywindow.cpp
#include "Headers/ui/lobbywindow.h"
#include "Headers/network/protocol_defines.h"
#include <QVBoxLayout>
#include <QMessageBox>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDebug>

LobbyWindow::LobbyWindow(const QString& username, ClientNetwork* network, QWidget *parent)
    : QWidget(parent), m_username(username), m_network(network)
{
    qDebug() << "LobbyWindow: Constructor called for user:" << username;
    QVBoxLayout *layout = new QVBoxLayout(this);

    m_lobbyList = new QListWidget(this);
    m_createBtn = new QPushButton("Создать лобби", this);
    m_joinBtn = new QPushButton("Присоединиться", this);

    layout->addWidget(m_lobbyList);
    layout->addWidget(m_createBtn);
    layout->addWidget(m_joinBtn);

    connect(m_createBtn, &QPushButton::clicked, this, &LobbyWindow::onCreateClicked);
    connect(m_joinBtn, &QPushButton::clicked, this, &LobbyWindow::onJoinClicked);
    // Подключаем обработчик сообщений от сети
    connect(m_network, &ClientNetwork::messageReceived, this, &LobbyWindow::handleNetworkMessage);

    // Запрос списка лобби при инициализации
    requestLobbyList();
}

void LobbyWindow::requestLobbyList() {
    qDebug() << "LobbyWindow: Requesting lobby list.";
    QJsonObject request;
    request[JsonFields::TYPE] = MessageType::GET_LOBBIES;
    m_network->sendMessage(request);
}

void LobbyWindow::onCreateClicked()
{
    qDebug() << "LobbyWindow: Create lobby button clicked.";
    QJsonObject msg;
    msg[JsonFields::TYPE] = MessageType::CREATE_LOBBY;
    m_network->sendMessage(msg);
}

void LobbyWindow::onJoinClicked()
{
    qDebug() << "LobbyWindow: Join lobby button clicked.";
    auto selected = m_lobbyList->currentItem();
    if (!selected) {
        QMessageBox::warning(this, "Ошибка", "Выберите лобби из списка");
        return;
    }

    QJsonObject msg;
    msg[JsonFields::TYPE] = MessageType::JOIN_LOBBY;
    msg[JsonFields::ROOM_ID] = selected->data(Qt::UserRole).toString();
    m_network->sendMessage(msg);
}

void LobbyWindow::handleNetworkMessage(const QJsonObject &message)
{
    int type = message[JsonFields::TYPE].toInt();
    qDebug() << "LobbyWindow: Received network message type:" << type;

    switch (type) {
    case MessageType::LOBBY_LIST:
        updateLobbyList(message);
        break;
    case MessageType::LOBBY_CREATED:
        QMessageBox::information(this, "Лобби создано", "Вы успешно создали лобби с ID: " + message[JsonFields::ROOM_ID].toString());
        requestLobbyList();
        break;
    case MessageType::JOIN_RESULT:
        if (message["success"].toBool()) {
            QMessageBox::information(this, "Присоединение", "Вы успешно присоединились к лобби.");
        } else {
            QMessageBox::warning(this, "Присоединение", "Не удалось присоединиться к лобби.");
            requestLobbyList();
        }
        break;
    case MessageType::GAME_START:
        qDebug() << "LobbyWindow: Game started signal emitted.";
        emit gameStarted(message);
        break;
    case MessageType::ROLE_ASSIGNED:
        qDebug() << "LobbyWindow: Role assigned:" << message[JsonFields::ROLE].toString();
        break;
    default:
        break;
    }
}

// FileName: /lobbywindow.cpp
void LobbyWindow::updateLobbyList(const QJsonObject &message)
{
    m_lobbyList->clear(); // <-- Очистка списка
    QJsonArray lobbies = message[JsonFields::LOBBY_LIST].toArray();
    qDebug() << "LobbyWindow: Updating lobby list with" << lobbies.size() << "lobbies.";

    for (const auto &lobby : lobbies) {
        QJsonObject obj = lobby.toObject();
        QListWidgetItem *item = new QListWidgetItem(
            QString("Лобби #%1 (%2/2 игроков) - Владелец: %3")
                .arg(obj[JsonFields::ROOM_ID].toString().left(6))
                .arg(obj["players_count"].toInt())
                .arg(obj[JsonFields::OWNER].toString())
            );
        item->setData(Qt::UserRole, obj[JsonFields::ROOM_ID].toString());
        m_lobbyList->addItem(item); // <-- Добавление элемента
    }
}


void LobbyWindow::resetLobbyState()
{
    qDebug() << "LobbyWindow: Resetting lobby state.";
    m_lobbyList->clear();
    requestLobbyList();
}
