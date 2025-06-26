#ifndef TST_MINESWEEPER_H
#define TST_MINESWEEPER_H

#include <QCoreApplication>
#include <QtTest>

class TestMineSweeper : public QObject
{
    Q_OBJECT
public:
    TestMineSweeper();

private slots:
    void testServerStart();
    void testGameLogic();
    void testPlayerConnection();
};

#endif // TST_MINESWEEPER_H
