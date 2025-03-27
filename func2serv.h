#ifndef FUNC2SERV_H
#define FUNC2SERV_H
#include<QByteArray>
#include <QString>
#include <QList>

QByteArray parse(QByteArray data, int id);
QByteArray auth(QString log, QString pas, int id);
QByteArray reg(QString log, QString pas, QString email, int id);
QByteArray getStat(QString log, int id);
QByteArray checkAnswer(int task_number, QString variant, QString answer);

#endif // FUNC2SERV_H
