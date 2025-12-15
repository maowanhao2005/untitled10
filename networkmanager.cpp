#include "networkmanager.h"
#include <QHostAddress>
#include <QNetworkInterface>
#include <QDebug>

NetworkManager::NetworkManager(QObject *parent, const QString &username)
    : QObject(parent), localUsername(username) {

    // 获取本地IP
    localIP = QHostAddress(QHostAddress::LocalHost).toString();
    auto interfaces = QNetworkInterface::allInterfaces();

    qDebug() << "=== 网络接口信息 ===";
    for (const auto &interface : interfaces) {
        qDebug() << "接口:" << interface.humanReadableName();
        qDebug() << "  状态: 上线" << interface.flags().testFlag(QNetworkInterface::IsUp)
                 << "运行中" << interface.flags().testFlag(QNetworkInterface::IsRunning)
                 << "回环" << interface.flags().testFlag(QNetworkInterface::IsLoopBack);

        if (interface.flags().testFlag(QNetworkInterface::IsUp) &&
            interface.flags().testFlag(QNetworkInterface::IsRunning) &&
            !interface.flags().testFlag(QNetworkInterface::IsLoopBack)) {
            for (const auto &addressEntry : interface.addressEntries()) {
                QHostAddress addr = addressEntry.ip();
                if (addr.protocol() == QAbstractSocket::IPv4Protocol && !addr.isLoopback()) {
                    localIP = addr.toString();
                    qDebug() << "  使用IP:" << localIP;
                    qDebug() << "  广播地址:" << addressEntry.broadcast().toString();
                    break;
                }
            }
        }
    }
    qDebug() << "最终本地IP:" << localIP;

    // 初始化UDP发现
    udpDiscovery = new UDPDiscovery(this, localIP, localUsername);
    connect(udpDiscovery, &UDPDiscovery::packetReceived, this, &NetworkManager::onUDPPacketReceived);

    // 初始化TCP服务器
    tcpServer = new TCPServer(this, chatPort);
    connect(tcpServer, &TCPServer::messageReceived, this, &NetworkManager::onTCPMessageReceived);

    qDebug() << "NetworkManager初始化完成";
    qDebug() << "用户名:" << localUsername;
    qDebug() << "聊天端口:" << chatPort;
}

void NetworkManager::sendMessageToAllPeers(const QString &message) {
    if (peers.isEmpty()) {
        qDebug() << "警告: 没有在线用户，消息未发送";
        return;
    }

    QString fullMessage = QString("[%1]: %2").arg(localUsername).arg(message);
    qDebug() << "发送消息到" << peers.size() << "个用户";

    for (auto it = peers.begin(); it != peers.end(); ++it) {
        QString peerIP = it.value().ip;
        QString peerName = it.value().username;

        if (peerIP == localIP) {
            qDebug() << "跳过自己:" << peerName;
            continue;
        }

        qDebug() << "  发送给:" << peerName << "(" << peerIP << ")";
        TCPClient *client = new TCPClient(this);
        connect(client, &TCPClient::destroyed, client, &QObject::deleteLater);
        client->sendMessage(peerIP, chatPort, fullMessage);
    }
}

void NetworkManager::onUDPPacketReceived(const QString &ip, const QString &username) {
    qDebug() << "UDP发现新节点: IP =" << ip << "用户名 =" << username;

    if (ip == localIP) {
        qDebug() << "忽略自己的广播";
        return;
    }

    if (!peers.contains(ip)) {
        PeerInfo peer;
        peer.ip = ip;
        peer.username = username;
        peer.port = chatPort;
        peers[ip] = peer;

        qDebug() << "新用户加入列表:" << username << "(" << ip << ")";
        emit peerDiscovered(ip, username);
    } else {
        qDebug() << "用户已存在:" << username;
    }
}

void NetworkManager::onTCPMessageReceived(const QString &message) {
    qDebug() << "收到TCP消息:" << message;
    emit messageReceived(message);
}