#include "networkmanager.h"
#include <QHostAddress>
#include <QNetworkInterface>
#include <QDebug>
#include <QTcpServer>
#include <QTcpSocket>

NetworkManager::NetworkManager(QObject *parent, const QString &username)
    : QObject(parent), localUsername(username), isServer(false) {

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

    // 初始化文件服务器
    fileServer = new QTcpServer(this);
    connect(fileServer, &QTcpServer::newConnection, this, &NetworkManager::handleNewConnection);

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

// 新增方法：发送文件到服务器
void NetworkManager::sendFileToServer(const QString &fileMessage) {
    // 查找服务器节点（假设第一个节点是服务器）
    if (!peers.isEmpty()) {
        auto serverIt = peers.begin();
        QString serverIP = serverIt.value().ip;

        TCPClient *client = new TCPClient(this);
        connect(client, &TCPClient::destroyed, client, &QObject::deleteLater);
        client->sendMessage(serverIP, chatPort, "[FILE_FORWARD]:" + fileMessage);
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
    if (message.startsWith("[FILE_FORWARD]:")) {
        // 这是转发的文件消息
        QString fileMessage = message.mid(15); // 移除"[FILE_FORWARD]:"前缀
        emit fileReceived(fileMessage);
    } else {
        qDebug() << "收到TCP消息:" << message;
        emit messageReceived(message);
    }
}

// 新增方法
void NetworkManager::setIsServer(bool server) {
    isServer = server;
    if (isServer) {
        if (fileServer->listen(QHostAddress::Any, 12347)) {
            qDebug() << "文件服务器启动在端口: 12347";
        } else {
            qCritical() << "无法启动文件服务器:" << fileServer->errorString();
        }
    }
}

bool NetworkManager::getIsServer() const {
    return isServer;
}

// 新增槽函数：处理新连接
void NetworkManager::handleNewConnection() {
    QTcpSocket *clientSocket = fileServer->nextPendingConnection();
    clients.append(clientSocket);
    connect(clientSocket, &QTcpSocket::readyRead, this, &NetworkManager::handleClientData);
    connect(clientSocket, &QTcpSocket::disconnected, [=]() {
        clients.removeAll(clientSocket);
        clientSocket->deleteLater();
    });
}

// 新增槽函数：处理客户端数据
void NetworkManager::handleClientData() {
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (!clientSocket)
        return;

    QByteArray data = clientSocket->readAll();
    QString message = QString::fromUtf8(data);

    if (message.startsWith("[FILE_FORWARD]:")) {
        // 这是从客户端发来的文件，需要转发给所有其他客户端
        QString fileMessage = message.mid(15); // 移除"[FILE_FORWARD]:"前缀

        // 转发给所有连接的客户端（除了发送者）
        for (QTcpSocket *socket : clients) {
            if (socket != clientSocket) {
                socket->write(("[FILE_FORWARD]:" + fileMessage).toUtf8());
            }
        }

        // 同时在本地触发文件接收信号
        emit fileReceived(fileMessage);
    }
}