#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenuBar>
#include <QAction>
#include <QMessageBox>
#include "Headers/network/clientnetwork.h"
#include "authwindow.h"
#include "lobbywindow.h"
#include "gamewindow.h"

class AuthWindow;
class LobbyWindow;
class GameWindow;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    void handleNetworkMessage(const QJsonObject& response);
    ~MainWindow();

private slots:
    void showAuthWindow();
    void showLobbyWindow();
    void exitApplication();

    void handleAuthResponse(const QJsonObject& response);
    void handleGameStarted(const QJsonObject& gameInfo);
    void handleReturnToLobby();

private:
    void setupMainMenu();
    void showError(const QString& message);

    AuthWindow* m_authWindow;
    LobbyWindow* m_lobbyWindow;
    GameWindow* m_gameWindow;
    ClientNetwork* m_network;
    QString m_username;
};

#endif // MAINWINDOW_H
