
#ifndef DATABASE_H
#define DATABASE_H

#include <QString>
#include <QHash>
#include <QList>

class Database {
public:
    Database();
    bool authenticate(const QString &username, const QString &password);
    bool registerUser(const QString &username, const QString &password, const QString &email);
    QString getStatistics(const QString &username);
    bool checkAnswer(int task_number, const QString &variant, const QString &answer);

private:
    QHash<QString, QString> users; // Хранение пользователей: username -> password
    QHash<QString, QString> emails; // Хранение email-ов: username -> email
};

#endif // DATABASE_H
