#ifndef GAME_MODEL_H
#define GAME_MODEL_H

#include <QObject>
#include <QVector>

class game_model : public QObject {
    Q_OBJECT
public:
    explicit game_model(int width = 10, int height = 10, int mines = 10, QObject *parent = nullptr);

    void startNewGame();
    int openCell(int x, int y);
    bool toggleFlag(int x, int y);
    bool checkWinCondition() const;

    int getWidth() const { return width; }
    int getHeight() const { return height; }
    int getMinesCount() const { return minesCount; }

signals:
    void gameStarted();
    void cellUpdated(int x, int y, int value);
    void flagToggled(int x, int y, bool flagged);
    void gameOver(bool mineHit);
    void gameWon();

private:
    void placeMines();

    int width;
    int height;
    int minesCount;
    QVector<QVector<int>> field;
    QVector<QVector<bool>> openedCells;
    QVector<QVector<bool>> flaggedCells;
};

#endif // GAME_MODEL_H
