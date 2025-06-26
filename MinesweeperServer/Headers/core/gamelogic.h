#ifndef GAMELOGIC_H
#define GAMELOGIC_H

#include <QObject>
#include <QVector>
#include <QPair>

class GameLogic : public QObject
{
    Q_OBJECT

public:
    enum CellState {
        Closed,       // Клетка закрыта
        Opened,       // Клетка открыта
        Flagged,      // Клетка помечена флагом
        QuestionMark  // Клетка с вопросом
    };

    enum PlayerRole {
        Scout,        // Сапер-разведчик
        Saboteur      // Сапер-диверсант
    };

    struct Cell {
        CellState state = Closed;
        bool hasMine = false;
        int minesAround = 0;
    };

    explicit GameLogic(QObject *parent = nullptr);
    void initializeGame(int rows, int cols, int minesCount);
    void resetGame();

    // Основные игровые действия
    bool openCell(int row, int col, PlayerRole player);
    bool toggleCellMark(int row, int col);
    bool switchPlayerTurn();

    // Получение информации о состоянии игры
    int rows() const { return m_rows; }
    int cols() const { return m_cols; }
    int totalMines() const { return m_minesCount; }
    int remainingMines() const { return m_remainingMines; }
    PlayerRole currentPlayer() const { return m_currentPlayer; }
    const QVector<QVector<Cell>>& gameField() const { return m_gameField; }
    bool isGameOver() const { return m_gameOver; }
    bool isGameWon() const { return m_gameWon; }
    bool checkWinCondition() const;

    // Информация о счете
    int scoutScore() const { return m_scoutScore; }
    int saboteurScore() const { return m_saboteurScore; }

signals:
    void gameFieldChanged();
    void gameOver(bool won);
    void playerSwitched(PlayerRole newPlayer);
    void scoresChanged(int scoutScore, int saboteurScore);

private:
    void generateMines(int firstClickRow, int firstClickCol);
    void calculateMinesAround();
    void revealEmptyArea(int row, int col);
    bool isValidCell(int row, int col) const;


    // Параметры игры
    int m_rows = 10;
    int m_cols = 10;
    int m_minesCount = 15;
    int m_remainingMines = 15;

    // Состояние игры
    bool m_gameOver = false;
    bool m_gameWon = false;
    bool m_firstClick = true;

    // Игровая механика
    PlayerRole m_currentPlayer = Scout;
    int m_scoutScore = 0;
    int m_saboteurScore = 0;

    // Игровое поле
    QVector<QVector<Cell>> m_gameField;
};

#endif // GAMELOGIC_H
