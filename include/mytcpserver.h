/**
 * @file mytcpserver.h
 * @brief Заголовочный файл для TCP-сервера.
 */

#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QtNetwork>
#include <QByteArray>
#include <QDebug>
#include <QMap>
#include <QList>
#include <QString>

/**
 * @class MyTcpServer
 * @brief Класс, реализующий TCP-сервер для обработки клиентских подключений.
 */
class MyTcpServer : public QObject
{
    Q_OBJECT
public:
    explicit MyTcpServer(QObject *parent = nullptr);
    ~MyTcpServer();

public slots:
    void slotNewConnection();
    void slotClientDisconnected();
    void slotServerRead();

private:
    QTcpServer *mTcpServer; /**< Указатель на TCP-сервер */
    QMap<int, QTcpSocket*> mTcpSocket; /**< Список активных клиентских сокетов */
};

#endif // MYTCPSERVER_H
