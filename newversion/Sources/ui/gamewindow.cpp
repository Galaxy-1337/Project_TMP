// FileName: /gamewindow.cpp
#include "headers/ui/gamewindow.h"
#include "Headers/network/protocol_defines.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonArray>
#include <QToolTip>
#include <QDebug>
#include <QRegularExpression>

GameWindow::GameWindow(const QJsonObject& gameInfo, ClientNetwork* network, QWidget* parent)
    : QWidget(parent),
    m_network(network),
    m_rows(gameInfo.value(JsonFields::ROWS).toInt()),
    m_cols(gameInfo.value(JsonFields::COLS).toInt())
{
    qDebug() << "GameWindow: Constructor called.";
    // Определение роли игрока через value()
    QString roleStr = gameInfo.value(JsonFields::ROLE).toString();
    m_playerRole = (roleStr == JsonFields::ROLE_SCOUT) ? Scout : Saboteur;
    m_currentTurn = Scout; // Изначально ход всегда у Разведчика

    setupUI();
    connect(m_network, &ClientNetwork::messageReceived,
            this, &GameWindow::handleNetworkMessage,
            Qt::QueuedConnection); // Используем QueuedConnection для безопасности
}

void GameWindow::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Информационная панель
    QHBoxLayout* infoLayout = new QHBoxLayout();
    m_statusLabel = new QLabel("Статус: ожидание начала игры", this);
    m_scoreLabel = new QLabel("Счёт: Разведчик 0 - 0 Диверсант", this);
    infoLayout->addWidget(m_statusLabel);
    infoLayout->addStretch();
    infoLayout->addWidget(m_scoreLabel);

    // Игровое поле
    m_grid = new QGridLayout();
    m_grid->setHorizontalSpacing(1);
    m_grid->setVerticalSpacing(1);

    m_cells.resize(m_rows);
    for (int i = 0; i < m_rows; ++i) {
        m_cells[i].resize(m_cols);
        for (int j = 0; j < m_cols; ++j) {
            QPushButton* cell = new QPushButton(this);
            cell->setFixedSize(32, 32);
            cell->setProperty(JsonFields::ROW, i);
            cell->setProperty(JsonFields::COL, j);

            connect(cell, &QPushButton::clicked, this, [this, cell]() {
                handleCellClick(cell->property(JsonFields::ROW).toInt(),
                                cell->property(JsonFields::COL).toInt());
            });

            m_cells[i][j] = cell;
            m_grid->addWidget(cell, i, j);
        }
    }

    // Кнопка сдаться
    m_surrenderBtn = new QPushButton("Сдаться", this);
    connect(m_surrenderBtn, &QPushButton::clicked, this, &GameWindow::surrender);

    // Компоновка
    mainLayout->addLayout(infoLayout);
    mainLayout->addLayout(m_grid);
    mainLayout->addWidget(m_surrenderBtn, 0, Qt::AlignCenter);
    setLayout(mainLayout);

    // Обновляем статус, чтобы показать роль игрока
    QString roleName = (m_playerRole == Scout) ? "Разведчик" : "Диверсант";
    m_statusLabel->setText(QString("Вы: %1. Ожидание хода...").arg(roleName));
}

void GameWindow::handleNetworkMessage(const QJsonObject& msg)
{
    qDebug() << "GameWindow: Received network message type:" << msg[JsonFields::TYPE].toInt();
    if (!msg.contains(JsonFields::TYPE)) return;

    int type = msg[JsonFields::TYPE].toInt();

    switch (type) {
    case MessageType::GAME_UPDATE:
        if (msg.contains(JsonFields::FIELD) &&
            msg.contains(JsonFields::CURRENT_TURN) &&
            msg.contains(JsonFields::SCOUT_SCORE) &&
            msg.contains(JsonFields::SABOTEUR_SCORE))
        {
            updateGameField(msg[JsonFields::FIELD].toArray());
            updateCurrentTurn(msg[JsonFields::CURRENT_TURN].toString());
            updateScores(msg[JsonFields::SCOUT_SCORE].toInt(),
                         msg[JsonFields::SABOTEUR_SCORE].toInt());
        }
        break;

    case MessageType::GAME_OVER:
        if (msg.contains(JsonFields::WINNER)) {
            showGameResult(msg[JsonFields::WINNER].toString());
        }
        break;

    case MessageType::ROLE_ASSIGNED:
        // Это сообщение должно прийти только один раз при начале игры
        if (msg.contains(JsonFields::ROLE)) {
            QString roleStr = msg[JsonFields::ROLE].toString();
            m_playerRole = (roleStr == JsonFields::ROLE_SCOUT) ? Scout : Saboteur;
            QString roleName = (m_playerRole == Scout) ? "Разведчик" : "Диверсант";
            m_statusLabel->setText(QString("Вы: %1. Ожидание хода...").arg(roleName));
            qDebug() << "GameWindow: Your role: " << roleName;
        }
        break;
    }
}

void GameWindow::updateGameField(const QJsonArray& field) {
    for (const auto& cellVal : field) {
        auto cell = cellVal.toObject();
        int r = cell[JsonFields::ROW].toInt();
        int c = cell[JsonFields::COL].toInt();
        int state = cell[JsonFields::STATE].toInt();
        bool hasMine = cell[JsonFields::HAS_MINE].toBool();
        QPushButton* btn = m_cells[r][c];
        btn->setEnabled(state != Opened);
        // Сброс стилей
        btn->setStyleSheet("");
        btn->setText("");
        // Обновляем отображение клетки
        switch (state) {
        case Opened:
            if (hasMine) {
                btn->setText("💣");
                btn->setStyleSheet("background: red; color: white;");
            } else if (cell[JsonFields::MINES_AROUND].toInt() > 0) {
                btn->setText(QString::number(cell[JsonFields::MINES_AROUND].toInt()));
                btn->setStyleSheet("background: lightgray; color: black;");
            } else {
                btn->setStyleSheet("background: lightgray;");
            }
            break;
        case Flagged:
            btn->setText("🚩");
            btn->setStyleSheet("background: yellow;");
            break;
        case QuestionMark:
            btn->setText("❓");
            btn->setStyleSheet("background: lightblue;");
            break;
        default: // Closed
            // УДАЛИТЬ ЭТОТ БЛОК:
            // if (m_playerRole == Saboteur && hasMine) {
            //     btn->setText("💣"); // Диверсант видит мины
            //     btn->setStyleSheet("background: purple; color: white;");
            // } else {
            //     btn->setStyleSheet("background: gray;");
            // }
            // ЗАМЕНИТЬ НА:
            btn->setStyleSheet("background: gray;"); // Все закрытые клетки выглядят одинаково
            break;
        }
    }
}

void GameWindow::updateCurrentTurn(const QString& currentTurnRole) {
    m_currentTurn = (currentTurnRole == JsonFields::ROLE_SCOUT) ? Scout : Saboteur;

    bool myTurn = (m_currentTurn == m_playerRole);
    QString roleName = (m_playerRole == Scout) ? "Разведчик" : "Диверсант";
    QString currentTurnName = (m_currentTurn == Scout) ? "Разведчик" : "Диверсант";

    m_statusLabel->setText(QString("Вы: %1. Ход: %2 (%3)")
                               .arg(roleName)
                               .arg(currentTurnName)
                               .arg(myTurn ? "Ваш" : "Соперника"));

    // Обновляем стили кнопок для индикации хода
    for (int i = 0; i < m_rows; ++i) {
        for (int j = 0; j < m_cols; ++j) {
            QPushButton* btn = m_cells[i][j];
            // Сохраняем текущий стиль, чтобы не перезаписывать стили Opened/Flagged/QuestionMark
            QString baseStyle = btn->styleSheet();
            if (baseStyle.contains("border")) {
                baseStyle.remove(QRegularExpression("border: [^;]*;"));
            }

            if (myTurn && btn->isEnabled()) {
                btn->setStyleSheet(baseStyle + "border: 2px solid green;");
            } else {
                btn->setStyleSheet(baseStyle);
            }
        }
    }
}

void GameWindow::handleCellClick(int row, int col)
{
    if (m_currentTurn != m_playerRole) {
        QToolTip::showText(QCursor::pos(), "Сейчас не ваш ход!", this);
        return;
    }

    QJsonObject msg;
    msg[JsonFields::TYPE] = MessageType::GAME_MOVE;
    msg[JsonFields::ROW] = row;
    msg[JsonFields::COL] = col;

    m_network->sendMessage(msg);
}

void GameWindow::updateScores(int scoutScore, int saboteurScore) {
    m_scoreLabel->setText(QString("Счёт: Разведчик %1 - %2 Диверсант")
                              .arg(scoutScore).arg(saboteurScore));
}

void GameWindow::showGameResult(const QString& winner) {
    QString resultText;
    if (winner == JsonFields::ROLE_SCOUT) {
        resultText = "Победил Разведчик!";
    } else if (winner == JsonFields::ROLE_SABOTEUR) {
        resultText = "Победил Диверсант!";
    } else {
        resultText = "Игра окончена. Неизвестный победитель.";
    }

    QMessageBox::information(this, "Игра окончена", resultText);
    emit returnToLobby();
}

void GameWindow::surrender() {
    if (QMessageBox::question(this, "Подтверждение",
                              "Вы уверены, что хотите сдаться? Это приведет к поражению.") == QMessageBox::Yes)
    {
        QJsonObject msg;
        msg[JsonFields::TYPE] = MessageType::SURRENDER;
        m_network->sendMessage(msg);
    }
}

GameWindow::~GameWindow() {
    qDebug() << "GameWindow: Destructor called.";
    disconnect(m_network, &ClientNetwork::messageReceived, this, &GameWindow::handleNetworkMessage);
}
