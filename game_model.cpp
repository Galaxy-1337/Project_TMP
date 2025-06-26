#include "game_model.h"
#include <QTime>
#include <QDebug>
#include <cstdlib>

game_model::game_model(int width, int height, int mines, QObject *parent)
    : QObject(parent), width(width), height(height), minesCount(mines) {
    field = QVector<QVector<int>>(width, QVector<int>(height, 0));
    openedCells = QVector<QVector<bool>>(width, QVector<bool>(height, false));
    flaggedCells = QVector<QVector<bool>>(width, QVector<bool>(height, false));
}

void game_model::startNewGame() {
    field.fill(QVector<int>(height, 0));
    openedCells.fill(QVector<bool>(height, false));
    flaggedCells.fill(QVector<bool>(height, false));
    placeMines();
    emit gameStarted();
}

void game_model::placeMines() {
    QTime time = QTime::currentTime();
    srand((uint)time.msec());

    int placedMines = 0;
    while (placedMines < minesCount) {
        int x = rand() % width;
        int y = rand() % height;
        if (field[x][y] != -1) {
            field[x][y] = -1;
            placedMines++;

            for (int dx = -1; dx <= 1; ++dx) {
                for (int dy = -1; dy <= 1; ++dy) {
                    int nx = x + dx;
                    int ny = y + dy;
                    if (nx >= 0 && nx < width && ny >= 0 && ny < height && field[nx][ny] != -1) {
                        field[nx][ny]++;
                    }
                }
            }
        }
    }
}

int game_model::openCell(int x, int y) {
    if (x < 0 || x >= width || y < 0 || y >= height || openedCells[x][y] || flaggedCells[x][y]) {
        return -2; // Недопустимое действие
    }

    openedCells[x][y] = true;
    int value = field[x][y];
    emit cellUpdated(x, y, value);

    if (value == -1) {
        emit gameOver(true);
    } else if (checkWinCondition()) {
        emit gameWon();
    }

    return value;
}

bool game_model::toggleFlag(int x, int y) {
    if (x < 0 || x >= width || y < 0 || y >= height || openedCells[x][y]) {
        return false;
    }

    flaggedCells[x][y] = !flaggedCells[x][y];
    emit flagToggled(x, y, flaggedCells[x][y]);
    return flaggedCells[x][y];
}

bool game_model::checkWinCondition() const {
    int closedCells = 0;
    for (int x = 0; x < width; ++x) {
        for (int y = 0; y < height; ++y) {
            if (field[x][y] != -1 && !openedCells[x][y]) {
                closedCells++;
            }
        }
    }
    return closedCells == 0;
}
