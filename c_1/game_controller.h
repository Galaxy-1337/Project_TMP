#ifndef GAME_CONTROLLER_H
#define GAME_CONTROLLER_H

#include <QObject>
#include <QTcpSocket>
#include "game_model.h"

class game_controller : public QObject {
    Q_OBJECT
public:
    explicit game_controller(game_model *model, QObject *parent = nullptr);

public slots:
    void connectToServer(const QString& name, const QString& sessionCode);
    void processCellClick(int x, int y, bool rightClick);

private slots:
    void readServerData();
    void handleDisconnected();

private:
    QTcpSocket *socket;
    game_model *model;
    QString playerName;
};

#endif // GAME_CONTROLLER_H