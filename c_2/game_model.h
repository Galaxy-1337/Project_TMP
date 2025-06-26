#ifndef GAME_MODEL_H
#define GAME_MODEL_H

#include <QObject>
#include <QVector>

class game_model : public QObject {
    Q_OBJECT
public:
    explicit game_model(QObject *parent = nullptr);
    void reset(int width, int height);
    void updateCell(int x, int y, int value);
    void setFlag(int x, int y, bool flagged);
    int getCellValue(int x, int y) const;
    bool isFlagged(int x, int y) const;
    bool isOpened(int x, int y) const;
    int getWidth() const { return width; }  // Добавлен геттер
    int getHeight() const { return height; } // Добавлен геттер

signals:
    void gameStarted();
    void cellUpdated(int x, int y);
    void flagUpdated(int x, int y);
    void gameOver(const QString &reason);
    void gameWon();

private:
    int width;
    int height;
    QVector<QVector<int>> field;
    QVector<QVector<bool>> openedCells;
    QVector<QVector<bool>> flaggedCells;
};

#endif // GAME_MODEL_H
