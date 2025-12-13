#include "udpdiscovery.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>

UDPDiscovery::UDPDiscovery(QObject *parent, const QString &localIP, const QString &username)
    : QObject(parent), localIP(localIP), username(username) {

    udpSocket = new QUdpSocket(this);
    udpSocket->bind(QHostAddress::Any, broadcastPort, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);

    connect(udpSocket, &QUdpSocket::readyRead, this, &UDPDiscovery::onReadyRead);

    broadcastTimer = new QTimer(this);
    connect(broadcastTimer, &QTimer::timeout, this, &UDPDiscovery::onBroadcastTimeout);
    broadcastTimer->start(5000); // 每5秒广播一次

    // 立即广播一次
    onBroadcastTimeout();
}

void UDPDiscovery::onReadyRead() {
    while (udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        QHostAddress senderIP;
        quint16 senderPort;

        datagram.resize(udpSocket->pendingDatagramSize());
        udpSocket->readDatagram(datagram.data(), datagram.size(), &senderIP, &senderPort);

        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(datagram, &error);
        if (error.error == QJsonParseError::NoError) {
            QJsonObject obj = doc.object();
            if (obj["type"].toString() == "online") {
                QString ip = obj["ip"].toString();
                QString username = obj["username"].toString();
                if (ip != this->localIP) { // 不接收自己的广播
                    emit packetReceived(ip, username);
                }
            }
        }
    }
}

void UDPDiscovery::onBroadcastTimeout() {
    QJsonObject obj;
    obj["type"] = "online";
    obj["ip"] = localIP;
    obj["username"] = username;
    QJsonDocument doc(obj);
    QByteArray data = doc.toJson(QJsonDocument::Compact);

    udpSocket->writeDatagram(data, QHostAddress::Broadcast, broadcastPort);
}