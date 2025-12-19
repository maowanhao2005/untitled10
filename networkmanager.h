#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QTcpSocket>
#include <QThread>
#include <QTcpServer>
#include <QList>
#include "udpdiscovery.h"
#include "tcpserver.h"
#include "tcpclient.h"

struct PeerInfo {
    QString ip;
    QString username;
    int port;
};

class NetworkManager : public QObject {
    Q_OBJECT

public:
    explicit NetworkManager(QObject *parent = nullptr, const QString &username = "");
    void sendMessageToAllPeers(const QString &message);
    void sendFileToServer(const QString &fileMessage);
    void setIsServer(bool server);
    bool getIsServer() const;

    signals:
        void messageReceived(const QString &message);
    void peerDiscovered(const QString &ip, const QString &username);
    void fileReceived(const QString &fileMessage);

private slots:
    void onUDPPacketReceived(const QString &ip, const QString &username);
    void onTCPMessageReceived(const QString &message);
    void handleNewConnection();
    void handleClientData();

private:
    QString localIP;
    QString localUsername;
    int chatPort = 12346;
    bool isServer = false;

    UDPDiscovery *udpDiscovery;
    TCPServer *tcpServer;
    QMap<QString, PeerInfo> peers;

    // 新增服务器相关成员
    QTcpServer *fileServer;
    QList<QTcpSocket*> clients;
};

#endif // NETWORKMANAGER_H