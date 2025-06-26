#include "tst_minesweeper.h"
#include "../Server/minesweeper_server.h"
#include <QTcpSocket>
#include <QTest>
#include <QDebug>  // Добавлен для qDebug()
#include <QCoreApplication>

TestMineSweeper::TestMineSweeper()
{
    qDebug() << "TestMineSweeper: Конструктор тестового класса";
}

void TestMineSweeper::testServerStart()
{
    qDebug() << "--- Запуск теста: testServerStart() ---";
    MineSweeperServer server(5555);
    qDebug() << "Сервер создан, порт: 5555";

    QVERIFY(server.isRunning());
    qDebug() << "Сервер запущен успешно";

    QVERIFY(server.tcpServer() != nullptr);
    qDebug() << "TCP сервер инициализирован корректно";
}

void TestMineSweeper::testGameLogic()
{
    qDebug() << "\n--- Запуск теста: testGameLogic() ---";
    MineSweeperServer server(5555);
    qDebug() << "Проверка начального состояния:";
    QCOMPARE(server.playerCount(), 0);
    qDebug() << "Количество игроков (ожидается 0):" << server.playerCount();

    QTcpSocket testSocket;
    qDebug() << "Попытка подключения к localhost:5555...";
    testSocket.connectToHost("localhost", 5555);

    if (testSocket.waitForConnected(1000)) {
        qDebug() << "Подключение установлено!";
        QTest::qWait(100);
        qDebug() << "Текущее количество игроков:" << server.playerCount();
        QCOMPARE(server.playerCount(), 1);

        testSocket.disconnectFromHost();
        qDebug() << "Инициировано отключение...";
        QVERIFY(testSocket.waitForDisconnected(1000));
        qDebug() << "Отключение завершено";
    } else {
        qDebug() << "Ошибка: не удалось подключиться к серверу";
    }
}

void TestMineSweeper::testPlayerConnection()
{
    qDebug() << "\n--- Запуск теста: testPlayerConnection() ---";
    MineSweeperServer server(5555);
    qDebug() << "Проверка работы сервера:";
    QVERIFY(server.isRunning());
    qDebug() << "Сервер активен";

    QTcpSocket socket;
    qDebug() << "Подключение нового клиента...";
    socket.connectToHost("localhost", 5555);

    QVERIFY2(socket.waitForConnected(1000), "Сервер не принимает подключения");
    qDebug() << "Клиент успешно подключен";
    QTest::qWait(100);

    qDebug() << "Текущее количество игроков:" << server.playerCount();
    QCOMPARE(server.playerCount(), 1);

    qDebug() << "Инициирование отключения...";
    socket.disconnectFromHost();
    QVERIFY(socket.waitForDisconnected(1000));
    qDebug() << "Клиент отключен";
    QTest::qWait(100);

    qDebug() << "Проверка количества игроков после отключения:" << server.playerCount();
    QCOMPARE(server.playerCount(), 0);
}

QTEST_APPLESS_MAIN(TestMineSweeper)
