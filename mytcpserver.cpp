#include "mytcpserver.h"
#include "func2serv.h"

#include <QDebug>
#include <QCoreApplication>
#include<QString>
#include <QMap>

MyTcpServer::~MyTcpServer()
{

    mTcpServer->close();
    //server_status=0;
}

MyTcpServer::MyTcpServer(QObject *parent) : QObject(parent){
    mTcpServer = new QTcpServer(this);

    connect(mTcpServer, &QTcpServer::newConnection,
            this, &MyTcpServer::slotNewConnection);

    if(!mTcpServer->listen(QHostAddress::Any, 23352)){
        qDebug() << "server is not started";
    } else {
        //server_status=1;
        qDebug() << "server is started";
    }
}

void MyTcpServer::slotNewConnection(){
    //if(server_status==1){
    qDebug()<<"slot new conn\n";
    QTcpSocket *curr_mTcpSocket = mTcpServer->nextPendingConnection();
    //curr_mTcpSocket = mTcpServer->nextPendingConnection();

    curr_mTcpSocket->write("Hello, World!!! I am echo server!\r\n");
    connect(curr_mTcpSocket, &QTcpSocket::readyRead,this,&MyTcpServer::slotServerRead);
    connect(curr_mTcpSocket,&QTcpSocket::disconnected,this,&MyTcpServer::slotClientDisconnected);

    mTcpSocket[curr_mTcpSocket->socketDescriptor()] = curr_mTcpSocket;
    //}
}

void MyTcpServer::slotServerRead(){
    QTcpSocket *curr_mTcpSocket = static_cast<QTcpSocket*>(sender());

    while(curr_mTcpSocket->bytesAvailable()>0)
    {
        QByteArray array =curr_mTcpSocket->readAll();
        qDebug()<<array<<"\n";
        qDebug() << "Received data:" << array;

        // Парсинг и обработка запроса
        QByteArray response = parse(array, curr_mTcpSocket->socketDescriptor());
        curr_mTcpSocket->write(response);
    }
}

void MyTcpServer::slotClientDisconnected() {
    QTcpSocket *curr_mTcpSocket = static_cast<QTcpSocket*>(sender());
    mTcpSocket.remove(curr_mTcpSocket->socketDescriptor());
    curr_mTcpSocket->close();
}
