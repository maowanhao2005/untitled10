#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QObject>
#include <QTcpSocket>

class TCPClient : public QObject {
    Q_OBJECT

public:
    explicit TCPClient(QObject *parent = nullptr);
    void sendMessage(const QString &ip, int port, const QString &message);


private slots:
    void onConnected();
    void onError();

private:
    QTcpSocket *socket;
    QString targetIP;
    int targetPort;
    QString messageToSend;
};

#endif // TCPCLIENT_H