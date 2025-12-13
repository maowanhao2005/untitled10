#include "networkmanager.h"
#include <QHostAddress>
#include <QNetworkInterface>

NetworkManager::NetworkManager(QObject *parent, const QString &username)
    : QObject(parent), localUsername(username) {

    // 获取本地IP
    localIP = QHostAddress(QHostAddress::LocalHost).toString();
    auto interfaces = QNetworkInterface::allInterfaces();
    for (const auto &interface : interfaces) {
        if (interface.flags().testFlag(QNetworkInterface::IsUp) &&
            interface.flags().testFlag(QNetworkInterface::IsRunning) &&
            !interface.flags().testFlag(QNetworkInterface::IsLoopBack)) {
            for (const auto &addressEntry : interface.addressEntries()) {
                QHostAddress addr = addressEntry.ip();
                if (addr.protocol() == QAbstractSocket::IPv4Protocol && !addr.isLoopback()) {
                    localIP = addr.toString();
                    break;
                }
            }
        }
    }

    // 初始化UDP发现
    udpDiscovery = new UDPDiscovery(this, localIP, localUsername);
    connect(udpDiscovery, &UDPDiscovery::packetReceived, this, &NetworkManager::onUDPPacketReceived);

    // 初始化TCP服务器
    tcpServer = new TCPServer(this, chatPort);
    connect(tcpServer, &TCPServer::messageReceived, this, &NetworkManager::onTCPMessageReceived);
}

// 在 networkmanager.cpp 中
void NetworkManager::sendMessageToAllPeers(const QString &message) {
    QString fullMessage = QString("[%1]: %2").arg(localUsername).arg(message);
    for (auto it = peers.begin(); it != peers.end(); ++it) {
        TCPClient *client = new TCPClient(this); // parent = this，避免内存泄漏
        connect(client, &TCPClient::destroyed, client, &QObject::deleteLater); // 可选，确保清理
        client->sendMessage(it.value().ip, chatPort, fullMessage);
    }
}

void NetworkManager::onUDPPacketReceived(const QString &ip, const QString &username) {
    if (ip != localIP && !peers.contains(ip)) {
        PeerInfo peer;
        peer.ip = ip;
        peer.username = username;
        peer.port = chatPort;
        peers[ip] = peer;

        emit peerDiscovered(ip, username);
    }
}

void NetworkManager::onTCPMessageReceived(const QString &message) {
    emit messageReceived(message);
}