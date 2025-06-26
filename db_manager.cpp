#include "db_manager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QStandardPaths>
#include <QDir>

DBManager::DBManager(QObject *parent) : QObject(parent) {
    initDatabase();
}

DBManager::~DBManager() {
    if (db.isOpen()) {
        db.close();
    }
}

bool DBManager::initDatabase() {
    QString dbPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    qDebug() << "Database path:" << dbPath;  // Добавьте эту строку

    QDir().mkpath(dbPath);
    dbPath += "/minesweeper.db";

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbPath);

    if (!db.open()) {
        qDebug() << "Error opening database:" << db.lastError().text();
        return false;
    }

    QSqlQuery query;
    query.exec("CREATE TABLE IF NOT EXISTS games ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "session_code TEXT NOT NULL, "
               "result TEXT NOT NULL, "
               "start_time DATETIME NOT NULL, "
               "end_time DATETIME NOT NULL, "
               "width INTEGER NOT NULL, "
               "height INTEGER NOT NULL, "
               "mines INTEGER NOT NULL)");

    return true;
}

void DBManager::logGame(const QString &sessionCode, const QString &result, const QDateTime &startTime, const QDateTime &endTime, int width, int height, int mines) {
    QSqlQuery query;
    query.prepare("INSERT INTO games (session_code, result, start_time, end_time, width, height, mines) "
                  "VALUES (:session_code, :result, :start_time, :end_time, :width, :height, :mines)");
    query.bindValue(":session_code", sessionCode);
    query.bindValue(":result", result);
    query.bindValue(":start_time", startTime);
    query.bindValue(":end_time", endTime);
    query.bindValue(":width", width);
    query.bindValue(":height", height);
    query.bindValue(":mines", mines);

    if (!query.exec()) {
        qDebug() << "Error logging game:" << query.lastError().text();
    }
}
