#ifndef UDPDISCOVERY_H
#define UDPDISCOVERY_H

#include <QObject>
#include <QUdpSocket>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>

class UDPDiscovery : public QObject {
    Q_OBJECT

public:
    explicit UDPDiscovery(QObject *parent = nullptr, const QString &localIP = "", const QString &username = "");

    signals:
        void packetReceived(const QString &ip, const QString &username);

private slots:
    void onReadyRead();
    void onBroadcastTimeout();

private:
    QUdpSocket *udpSocket;
    QTimer *broadcastTimer;
    QString localIP;
    QString username;
    static const int broadcastPort = 12345;
};

#endif // UDPDISCOVERY_H
