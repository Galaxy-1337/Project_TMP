#ifndef LOBBYWINDOW_H
#define LOBBYWINDOW_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include "Headers/network/clientnetwork.h"
#include "Headers/network/protocol_defines.h" // Убедитесь, что это включено

class LobbyWindow : public QWidget
{
    Q_OBJECT
public:
    explicit LobbyWindow(const QString& username, ClientNetwork* network, QWidget *parent = nullptr);
    void resetLobbyState();
    void requestLobbyList();
    void handleNetworkMessage(const QJsonObject &message);

signals:
    void gameStarted(const QJsonObject& gameInfo);

private slots:
    void onCreateClicked();
    void onJoinClicked();
    void updateLobbyList(const QJsonObject &message);

private:
    QString m_username;
    ClientNetwork* m_network;
    QListWidget *m_lobbyList;
    QPushButton *m_createBtn;
    QPushButton *m_joinBtn;
};

#endif // LOBBYWINDOW_H
