#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QList>

class TCPServer : public QTcpServer {
    Q_OBJECT


public:
    explicit TCPServer(QObject *parent = nullptr, int port = 0);

    signals:
        void messageReceived(const QString &message);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private:
    int serverPort;
};

class TCPConnectionHandler : public QObject {
    Q_OBJECT

public:
    explicit TCPConnectionHandler(qintptr socketDescriptor, QObject *parent = nullptr);

    signals:
        void messageReceived(const QString &message);

private slots:
    void onReadyRead();
    void onDisconnected();

private:
    QTcpSocket *socket;
};

#endif // TCPSERVER_H