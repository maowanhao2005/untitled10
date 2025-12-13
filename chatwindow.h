#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QLabel>
#include <QInputDialog>
#include <QTimer>
#include "networkmanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ChatWindow; }
QT_END_NAMESPACE

class ChatWindow : public QWidget {
    Q_OBJECT

public:
    explicit ChatWindow(QWidget *parent = nullptr);
    ~ChatWindow();

private slots:
    void onSendMessage();
    void onMessageReceived(const QString &message);
    void onPeerDiscovered(const QString &ip, const QString &username);

private:
    void setupUI();
    void setupConnections();

    QVBoxLayout *mainLayout;
    QTextEdit *chatHistory;
    QLineEdit *messageInput;
    QPushButton *sendButton;
    QListWidget *onlineUsersList;
    QLabel *statusLabel;

    NetworkManager *networkManager;
    QString username;
};

#endif // CHATWINDOW_H