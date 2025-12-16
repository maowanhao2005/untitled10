#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QTcpSocket>
#include <QThread>
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

signals:
    void messageReceived(const QString &message);
    void peerDiscovered(const QString &ip, const QString &username);

private slots:
    void onUDPPacketReceived(const QString &ip, const QString &username);
    void onTCPMessageReceived(const QString &message);

private:
    QString localIP;
    QString localUsername;
    int chatPort = 12346;

    UDPDiscovery *udpDiscovery;
    TCPServer *tcpServer;
    QMap<QString, PeerInfo> peers;
};

#endif // NETWORKMANAGER_H
