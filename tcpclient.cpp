#include "tcpclient.h"
#include <QTcpSocket>
#include <QTimer>

TCPClient::TCPClient(QObject *parent) : QObject(parent) {
    socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::connected, this, &TCPClient::onConnected);
    connect(socket, &QTcpSocket::errorOccurred, this, &TCPClient::onError);
}

void TCPClient::sendMessage(const QString &ip, int port, const QString &message) {
    targetIP = ip;
    targetPort = port;
    messageToSend = message;

    socket->connectToHost(ip, port);
}

void TCPClient::onConnected() {
    socket->write(messageToSend.toUtf8());
    socket->disconnectFromHost();
}

void TCPClient::onError() {
    qDebug() << "发送消息失败到" << targetIP << ":" << targetPort << "-" << socket->errorString();
    socket->disconnectFromHost();
}