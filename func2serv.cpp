#include "func2serv.h"
#include <QString>
#include <QByteArray>
#include <QList>

QByteArray parse(QByteArray data,int id){
    if (data.isEmpty()) {
        return "";
    }

    QList<QByteArray> splitParams = data.split('&');
    char requestType = data[0]; // Первый байт определяет тип запроса

    switch (requestType) {
    case '1':
        return auth(QString(splitParams[1]), QString(splitParams[2]), id); // auth
        break;
    case '2':
        return reg(QString(splitParams[1]), QString(splitParams[2]), QString(splitParams[3]), id); // reg
        break;
    case '3':
        return getStat(QString(splitParams[1]), id); // stat
        break;
    case '4':
        return checkAnswer(splitParams[1].toInt(), QString(splitParams[2]), QString(splitParams[3])); // check
        break;
    default:
        return "Неизвестный тип запроса\r\n";
        break;
    }
}

QByteArray auth(QString log, QString pas, int id) {
    QString role;
    role= log;
    if((log=="user" or log=="admin")&& pas=="123")
        return ("auth+&"+log+"&"+role).toUtf8();
    else {
        return "auth-";
    }
}

QByteArray reg(QString log, QString pas, QString email, int id) {
    return "reg+&" + log.toUtf8();
}

QByteArray getStat(QString log, int id) {
    if(log=="user"){
        return ("stat&3$6&21");}
    else {
        return "stat&0&3&-4";
    }
}

QByteArray checkAnswer(int task_number, QString variant, QString answer) {
    return (variant == "correct") ? "check+" : "check-"; // Заглушка для проверки ответа
}
