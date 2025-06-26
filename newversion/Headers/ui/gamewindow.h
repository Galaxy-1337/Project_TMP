#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H

#include <QWidget>
#include <QGridLayout>
#include <QVector>
#include <QLabel>
#include <QPushButton>
#include "Headers/network/clientnetwork.h"
#include "Headers/network/protocol_defines.h"

class ClientNetwork;
class QLabel;
class QPushButton;

class GameWindow : public QWidget
{
    Q_OBJECT
public:
    enum PlayerRole {
        Scout,
        Saboteur
    };

    enum CellState {
        Closed,
        Opened,
        Flagged,
        QuestionMark
    };

    explicit GameWindow(const QJsonObject& gameInfo, ClientNetwork* network, QWidget* parent = nullptr);
    ~GameWindow();

signals:
    void returnToLobby();

private slots:
    void handleNetworkMessage(const QJsonObject& msg);
    void handleCellClick(int row, int col);
    void surrender();

private:
    void setupUI();
    void updateGameField(const QJsonArray& field);
    void updateCurrentTurn(const QString& currentTurnRole);
    void updateScores(int scoutScore, int saboteurScore);
    void showGameResult(const QString& winner);

    ClientNetwork* m_network;
    PlayerRole m_playerRole;
    PlayerRole m_currentTurn;

    QGridLayout* m_grid;
    QVector<QVector<QPushButton*>> m_cells;
    QLabel* m_statusLabel;
    QLabel* m_scoreLabel;
    QPushButton* m_surrenderBtn;

    int m_rows;
    int m_cols;
};

#endif // GAMEWINDOW_H
