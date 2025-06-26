// FileName: /mainwindow.cpp
#include "Headers/ui/mainwindow.h"
#include "Headers/ui/authwindow.h"
#include "Headers/ui/lobbywindow.h"
#include "Headers/ui/gamewindow.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    m_authWindow(nullptr),
    m_lobbyWindow(nullptr),
    m_gameWindow(nullptr),
    m_network(new ClientNetwork(this))
{
    qDebug() << "MainWindow: Constructor called.";
    setWindowTitle("Minesweeper Game");

    resize(400, 300);
    setupMainMenu();
    // MainWindow теперь будет обрабатывать только сообщения, которые касаются
    // переключения окон (AUTH_RESPONSE, GAME_START)
    // Остальные сообщения будут обрабатываться напрямую в LobbyWindow или GameWindow
    connect(m_network, &ClientNetwork::messageReceived, this, &MainWindow::handleNetworkMessage);

    if (!m_network->connectToServer("192.168.0.101", 12345)) {
        showError("Failed to connect to server");
    }
}

MainWindow::~MainWindow()
{
    qDebug() << "MainWindow: Destructor called.";
}

void MainWindow::setupMainMenu()
{
    qDebug() << "MainWindow: Setting up main menu.";
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);
    QLabel *titleLabel = new QLabel("Minesweeper Game", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold;");
    QPushButton *loginButton = new QPushButton("Войти", this);
    QPushButton *lobbyButton = new QPushButton("Лобби", this);
    QPushButton *exitButton = new QPushButton("Выход", this);
    layout->addWidget(titleLabel);
    layout->addWidget(loginButton);
    layout->addWidget(lobbyButton);
    layout->addWidget(exitButton);
    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);
    connect(loginButton, &QPushButton::clicked, this, &MainWindow::showAuthWindow);
    connect(lobbyButton, &QPushButton::clicked, this, &MainWindow::showLobbyWindow);
    connect(exitButton, &QPushButton::clicked, this, &MainWindow::exitApplication);
}

void MainWindow::showAuthWindow()
{
    qDebug() << "MainWindow: Opening auth window.";

    if (!m_authWindow) {
        m_authWindow = new AuthWindow(this);
        m_authWindow->setWindowFlags(Qt::Window);
        m_authWindow->move(this->pos());

        connect(m_authWindow, &AuthWindow::authRequested, this, [this](const QString& login, const QString& passHash) {
            QJsonObject authMsg;
            authMsg[JsonFields::TYPE] = MessageType::AUTH_REQUEST;
            authMsg[JsonFields::LOGIN] = login;
            authMsg[JsonFields::PASSWORD_HASH] = passHash;
            qDebug() << "MainWindow: Sending authentication request.";
            m_network->sendMessage(authMsg);
        });

        connect(m_authWindow, &AuthWindow::registerRequested, this, [this](const QString& login, const QString& passHash) {
            QJsonObject regMsg;
            regMsg[JsonFields::TYPE] = MessageType::REGISTER_REQUEST;
            regMsg[JsonFields::LOGIN] = login;
            regMsg[JsonFields::PASSWORD_HASH] = passHash;
            qDebug() << "MainWindow: Sending registration request.";
            m_network->sendMessage(regMsg);
        });
    }

    this->hide();
    m_authWindow->show();
}


void MainWindow::showLobbyWindow()
{
    qDebug() << "MainWindow: showLobbyWindow called. Current username:" << m_username;
    if (m_username.isEmpty()) {
        qDebug() << "MainWindow: Username is empty, redirecting to AuthWindow.";
        QMessageBox::warning(this, "Ошибка", "Сначала необходимо войти в систему.");
        showAuthWindow();
        return;
    }

    if (!m_lobbyWindow) {
        m_lobbyWindow = new LobbyWindow(m_username, m_network, this);
        m_lobbyWindow->setWindowFlags(Qt::Window);
        connect(m_lobbyWindow, &LobbyWindow::gameStarted, this, &MainWindow::handleGameStarted);
    }
    this->hide();
    m_lobbyWindow->show();
    qDebug() << "MainWindow: LobbyWindow shown.";
}


void MainWindow::exitApplication()
{
    qDebug() << "MainWindow: Exiting application.";
    QApplication::exit();
}

void MainWindow::handleNetworkMessage(const QJsonObject& response)
{
    qDebug() << "MainWindow: Received network message type:" << response[JsonFields::TYPE].toInt();
    if (!response.contains(JsonFields::TYPE)) return;

    switch (response[JsonFields::TYPE].toInt()) {
    case MessageType::AUTH_RESPONSE:
        handleAuthResponse(response);
        break;

        // LOBBY_LIST и ROLE_ASSIGNED теперь обрабатываются напрямую в LobbyWindow
        // GAME_UPDATE, GAME_OVER, GAME_MOVE, SURRENDER теперь обрабатываются напрямую в GameWindow
        // MainWindow должен обрабатывать только те сообщения, которые вызывают смену окон
    case MessageType::GAME_START:
        handleGameStarted(response);
        break;
    default:
        // Если сообщение не для MainWindow, оно будет обработано активным окном
        // (LobbyWindow или GameWindow), так как они напрямую подключены к m_network->messageReceived
        qDebug() << "MainWindow: Message type" << response[JsonFields::TYPE].toInt() << "not handled by MainWindow. Assuming it's for active sub-window.";
        break;
    }
}

void MainWindow::handleAuthResponse(const QJsonObject& response)
{
    qDebug() << "MainWindow: Handling AUTH_RESPONSE. Success:" << response["success"].toBool();
    if (response["success"].toBool()) {
        m_username = response[JsonFields::USERNAME].toString();
        qDebug() << "MainWindow: Authentication successful. Username:" << m_username;
        if (m_authWindow) {
            m_authWindow->hide();
            qDebug() << "MainWindow: AuthWindow hidden.";
        }
        // Вместо showLobbyWindow()
        this->show(); // Показываем главное окно (меню)
        qDebug() << "MainWindow: Main window shown after successful auth.";
        // Опционально: можно сразу запросить список лобби, чтобы он был готов,
        // когда пользователь нажмет кнопку "Лобби".
        // if (m_lobbyWindow) {
        //     m_lobbyWindow->requestLobbyList();
        // }
    } else {
        qDebug() << "MainWindow: Authentication failed. Reason:" << response[JsonFields::MESSAGE].toString();
        showError(response[JsonFields::MESSAGE].toString());
    }
}


void MainWindow::handleGameStarted(const QJsonObject& gameInfo)
{
    qDebug() << "MainWindow: Game started. Game info:" << gameInfo;
    if (m_gameWindow) {
        m_gameWindow->deleteLater();
        m_gameWindow = nullptr;
    }
    m_gameWindow = new GameWindow(gameInfo, m_network, this);
    m_gameWindow->setWindowFlags(Qt::Window);
    connect(m_gameWindow, &GameWindow::returnToLobby, this, &MainWindow::handleReturnToLobby);
    if (m_lobbyWindow) {
        m_lobbyWindow->hide();
        qDebug() << "MainWindow: LobbyWindow hidden.";
    }
    m_gameWindow->show();
    qDebug() << "MainWindow: GameWindow shown.";
}

void MainWindow::handleReturnToLobby()
{
    qDebug() << "MainWindow: Returning to lobby.";
    if (m_lobbyWindow) {
        m_lobbyWindow->resetLobbyState();
        m_lobbyWindow->show();
        qDebug() << "MainWindow: LobbyWindow shown after game.";
    } else {
        // Если LobbyWindow не существует (например, приложение только что запустилось и сразу перешло в игру),
        // создаем его и показываем.
        showLobbyWindow();
    }
    if (m_gameWindow) {
        m_gameWindow->deleteLater();
        m_gameWindow = nullptr;
        qDebug() << "MainWindow: GameWindow deleted.";
    }
    this->show();
}

void MainWindow::showError(const QString& message)
{
    qWarning() << "MainWindow: Displaying error:" << message;
    QMessageBox::critical(this, "Error", message);
}
