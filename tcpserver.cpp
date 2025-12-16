#include "tcpserver.h"
#include <QTcpSocket>
#include <QDataStream>

TCPServer::TCPServer(QObject *parent, int port)
    : QTcpServer(parent), serverPort(port) {
    if (!this->listen(QHostAddress::Any, serverPort)) {
        qCritical() << "无法启动TCP服务器:" << this->errorString();
    } else {
        qDebug() << "TCP服务器启动在端口:" << serverPort;
    }
}


void TCPServer::incomingConnection(qintptr socketDescriptor) {
    TCPConnectionHandler *handler = new TCPConnectionHandler(socketDescriptor, this);
    connect(handler, &TCPConnectionHandler::messageReceived, this, &TCPServer::messageReceived);
}

TCPConnectionHandler::TCPConnectionHandler(qintptr socketDescriptor, QObject *parent)
    : QObject(parent) {
    socket = new QTcpSocket(this);
    socket->setSocketDescriptor(socketDescriptor);

    connect(socket, &QTcpSocket::readyRead, this, &TCPConnectionHandler::onReadyRead);
    connect(socket, &QTcpSocket::disconnected, this, &TCPConnectionHandler::onDisconnected);
}

void TCPConnectionHandler::onReadyRead() {
    QByteArray data = socket->readAll();
    QString message = QString::fromUtf8(data);
    emit messageReceived(message);
}

void TCPConnectionHandler::onDisconnected() {
    socket->deleteLater();
}