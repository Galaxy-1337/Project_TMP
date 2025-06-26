// db_manager.h
#ifndef DB_MANAGER_H
#define DB_MANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QDateTime>

class DBManager : public QObject {
    Q_OBJECT
public:
    explicit DBManager(QObject *parent = nullptr);
    ~DBManager();

    bool initDatabase();
    void logGame(const QString &sessionCode, const QString &result, const QDateTime &startTime, const QDateTime &endTime, int width, int height, int mines);

private:
    QSqlDatabase db;
};

#endif // DB_MANAGER_H
