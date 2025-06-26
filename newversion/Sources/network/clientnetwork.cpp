// FileName: /clientnetwork.cpp
#include "Headers/network/clientnetwork.h"
#include <QHostAddress>
#include <QJsonDocument>
#include <QDebug>
#include <QJsonParseError>

ClientNetwork::ClientNetwork(QObject *parent)
    : QObject(parent)
    , m_socket(new QTcpSocket(this))
{
    qDebug() << "ClientNetwork: Constructor called.";
    // Настройка соединений сигналов/слотов
    connect(m_socket, &QTcpSocket::readyRead, this, &ClientNetwork::onReadyRead);
    connect(m_socket, &QTcpSocket::stateChanged, this, &ClientNetwork::onStateChanged);
}

bool ClientNetwork::connectToServer(const QString &host, quint16 port)
{
    qDebug() << "ClientNetwork: Attempting to connect to server:" << host << ":" << port;
    if (m_socket->state() != QTcpSocket::UnconnectedState) {
        m_socket->abort();
    }
    m_socket->connectToHost(host, port);

    if (m_socket->waitForConnected(5000)) {
        qDebug() << "ClientNetwork: Connected to server:" << host << port;
        return true;
    } else {
        qWarning() << "ClientNetwork: Failed to connect to server:" << m_socket->errorString();
        return false;
    }
}

void ClientNetwork::sendMessage(const QJsonObject &message)
{
    if (m_socket->state() == QTcpSocket::ConnectedState) {
        QByteArray data = QJsonDocument(message).toJson(QJsonDocument::Compact) + "\n";
        m_socket->write(data);
        m_socket->flush();
        qDebug() << "ClientNetwork: Sent message:" << data;
    } else {
        qWarning() << "ClientNetwork: Cannot send message, socket not connected.";
    }
}

void ClientNetwork::onReadyRead()
{
    m_readBuffer.append(m_socket->readAll());
    qDebug() << "ClientNetwork: Raw data received:" << m_readBuffer;
    qDebug() << "ClientNetwork: Buffer after append:" << m_readBuffer;

    while (true) {
        int newlineIndex = m_readBuffer.indexOf('\n');
        qDebug() << "ClientNetwork: Newline index:" << newlineIndex;

        if (newlineIndex == -1) {
            // Нет полного сообщения в буфере
            break;
        }

        QByteArray messageData = m_readBuffer.left(newlineIndex);
        m_readBuffer.remove(0, newlineIndex + 1);
        qDebug() << "ClientNetwork: Parsing message data:" << messageData;
        qDebug() << "ClientNetwork: Buffer after remove:" << m_readBuffer;

        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(messageData, &error);

        if (error.error == QJsonParseError::NoError) {
            qDebug() << "ClientNetwork: Successfully parsed JSON object.";
            emit messageReceived(doc.object());
        } else {
            qWarning() << "ClientNetwork: JSON parse error:" << error.errorString() << "in data:" << messageData;
            // В случае ошибки парсинга, можно решить, что делать:
            // - Пропустить сообщение (как сейчас)
            // - Закрыть соединение
            // - Попытаться восстановиться (сложно)
        }
    }
}

void ClientNetwork::onStateChanged(QAbstractSocket::SocketState state)
{
    qDebug() << "ClientNetwork: Socket state changed to:" << state;
    emit connectionStateChanged(state == QTcpSocket::ConnectedState);
    if (state == QTcpSocket::UnconnectedState) {
        qDebug() << "ClientNetwork: Disconnected from server";
    }
}
