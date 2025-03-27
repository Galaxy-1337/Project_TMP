#include "database.h"

Database::Database() {
    // Эмулируем пользователей
    registerUser("user", "123", "user@example.com");
    registerUser("admin", "123", "admin@example.com");
}

bool Database::authenticate(const QString &username, const QString &password) {
    return users.contains(username) && users[username] == password;
}

bool Database::registerUser(const QString &username, const QString &password, const QString &email) {
    if (!users.contains(username)) {
        users[username] = password;
        emails[username] = email;
        return true; // Успешная регистрация
    }
    return false; // Пользователь уже существует
}

QString Database::getStatistics(const QString &username) {
    if (users.contains(username)) {
        // Эмулируем получение статистики для пользователя
        return "stat&3$6&21\r\nstat&0&3&-4\r\n";
    }
    return "stat&0&0&0\r\n"; // Если пользователь не найден
}

bool Database::checkAnswer(int task_number, const QString &variant, const QString &answer) {
    // Эмуляция проверки ответа
    return (task_number == 1 && answer == "p") || (task_number == 2 && answer == "correct_answer");
}
