#include "game_model.h"

game_model::game_model(QObject *parent) : QObject(parent), width(0), height(0) {}

void game_model::reset(int width, int height) {
    this->width = width;
    this->height = height;
    field = QVector<QVector<int>>(width, QVector<int>(height, -2));
    openedCells = QVector<QVector<bool>>(width, QVector<bool>(height, false));
    flaggedCells = QVector<QVector<bool>>(width, QVector<bool>(height, false));
    emit gameStarted();
}

void game_model::updateCell(int x, int y, int value) {
    if (x >= 0 && x < width && y >= 0 && y < height) {
        field[x][y] = value;
        openedCells[x][y] = true;
        emit cellUpdated(x, y);
    }
}

void game_model::setFlag(int x, int y, bool flagged) {
    if (x >= 0 && x < width && y >= 0 && y < height) {
        flaggedCells[x][y] = flagged;
        emit flagUpdated(x, y);
    }
}

int game_model::getCellValue(int x, int y) const {
    if (x >= 0 && x < width && y >= 0 && y < height) {
        return field[x][y];
    }
    return -2;
}

bool game_model::isFlagged(int x, int y) const {
    if (x >= 0 && x < width && y >= 0 && y < height) {
        return flaggedCells[x][y];
    }
    return false;
}

bool game_model::isOpened(int x, int y) const {
    if (x >= 0 && x < width && y >= 0 && y < height) {
        return openedCells[x][y];
    }
    return false;
}