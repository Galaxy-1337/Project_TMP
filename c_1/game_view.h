#ifndef GAME_VIEW_H
#define GAME_VIEW_H

#include <QMainWindow>
#include <QGridLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include "game_model.h"

class game_view : public QMainWindow {
    Q_OBJECT
public:
    explicit game_view(QWidget *parent = nullptr);
    void setModel(game_model *model);

signals:
    void connectRequested(const QString &name, const QString &sessionCode);
    void cellClicked(int x, int y, bool rightClick);

private slots:
    void setupConnectionUI();
    void setupGameUI();
    void updateCell(int x, int y);
    void updateFlag(int x, int y);
    void showGameStarted();
    void showGameOver(const QString &reason);
    void showGameWon();

private:
    game_model *model;
    QWidget *centralWidget;
    QLineEdit *nameEdit;
    QLineEdit *sessionCodeEdit;
    QPushButton *connectButton;
    QLabel *statusLabel;
    QGridLayout *gameGrid;
    QVector<QVector<QPushButton*>> cells;
};

#endif // GAME_VIEW_H
