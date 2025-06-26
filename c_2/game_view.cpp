#include "game_view.h"
#include <QVBoxLayout>
#include <QMessageBox>

game_view::game_view(QWidget *parent) : QMainWindow(parent), model(nullptr) {
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    setupConnectionUI();
}

void game_view::setModel(game_model *model) {
    this->model = model;
    connect(model, &game_model::cellUpdated, this, &game_view::updateCell);
    connect(model, &game_model::flagUpdated, this, &game_view::updateFlag);
    connect(model, &game_model::gameStarted, this, &game_view::showGameStarted);
    connect(model, &game_model::gameOver, this, &game_view::showGameOver);
    connect(model, &game_model::gameWon, this, &game_view::showGameWon);
}

void game_view::setupConnectionUI() {
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);
    nameEdit = new QLineEdit(this);
    nameEdit->setPlaceholderText("Enter your name");
    sessionCodeEdit = new QLineEdit(this);
    sessionCodeEdit->setPlaceholderText("Enter session code");
    connectButton = new QPushButton("Connect", this);
    statusLabel = new QLabel("Enter name and session code to connect", this);

    layout->addWidget(nameEdit);
    layout->addWidget(sessionCodeEdit);
    layout->addWidget(connectButton);
    layout->addWidget(statusLabel);

    connect(connectButton, &QPushButton::clicked, this, [=]() {
        emit connectRequested(nameEdit->text(), sessionCodeEdit->text());
    });
}

void game_view::setupGameUI() {
    delete centralWidget;
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);
    gameGrid = new QGridLayout();
    statusLabel = new QLabel("Game started!", this);

    cells.resize(model->getWidth());
    for (int x = 0; x < model->getWidth(); ++x) {
        cells[x].resize(model->getHeight());
        for (int y = 0; y < model->getHeight(); ++y) {
            QPushButton *cell = new QPushButton("", this);
            cell->setFixedSize(40, 40);
            connect(cell, &QPushButton::clicked, this, [=]() {
                emit cellClicked(x, y, false);
            });
            cell->setContextMenuPolicy(Qt::CustomContextMenu);
            connect(cell, &QPushButton::customContextMenuRequested, this, [=]() {
                emit cellClicked(x, y, true);
            });
            gameGrid->addWidget(cell, x, y);
            cells[x][y] = cell;
        }
    }

    layout->addLayout(gameGrid);
    layout->addWidget(statusLabel);
}

void game_view::updateCell(int x, int y) {
    if (!model) return;
    QPushButton *cell = cells[x][y];
    int value = model->getCellValue(x, y);
    if (value == -1) {
        cell->setText("💣");
    } else if (value > 0) {
        cell->setText(QString::number(value));
    } else {
        cell->setText("");
    }
    cell->setEnabled(false);
}

void game_view::updateFlag(int x, int y) {
    if (!model) return;
    QPushButton *cell = cells[x][y];
    cell->setText(model->isFlagged(x, y) ? "🚩" : "");
}

void game_view::showGameStarted() {
    setupGameUI();
    statusLabel->setText("Game started!");
}

void game_view::showGameOver(const QString& reason) {
    QString message = reason == "MINE" ? "Game Over: Mine hit!" : "Game Over: Player disconnected!";
    QMessageBox::information(this, "Game Over", message);
    statusLabel->setText(message);
}

void game_view::showGameWon() {
    QMessageBox::information(this, "Victory", "Congratulations! You won!");
    statusLabel->setText("Game Won!");
}
