#include "chatwindow.h"
#include <QApplication>
#include <QMessageBox>

ChatWindow::ChatWindow(QWidget *parent) : QWidget(parent) {
    setupUI();
    setupConnections();

    bool ok;
    username = QInputDialog::getText(this, "用户名", "请输入您的用户名:", QLineEdit::Normal, "", &ok);
    if (!ok || username.isEmpty()) {
        username = "匿名用户";
    }

    networkManager = new NetworkManager(this, username);
    connect(networkManager, &NetworkManager::messageReceived, this, &ChatWindow::onMessageReceived);
    connect(networkManager, &NetworkManager::peerDiscovered, this, &ChatWindow::onPeerDiscovered);

    statusLabel->setText(QString("就绪 - 用户名: %1").arg(username));
}

ChatWindow::~ChatWindow() {
    delete networkManager;
}

void ChatWindow::setupUI() {
    setWindowTitle("P2P 聊天室");
    resize(800, 600);

    mainLayout = new QVBoxLayout(this);

    // 聊天历史区域
    chatHistory = new QTextEdit(this);
    chatHistory->setReadOnly(true);
    mainLayout->addWidget(chatHistory);

    // 输入区域
    QHBoxLayout *inputLayout = new QHBoxLayout();
    messageInput = new QLineEdit(this);
    sendButton = new QPushButton("发送", this);
    inputLayout->addWidget(messageInput);
    inputLayout->addWidget(sendButton);
    mainLayout->addLayout(inputLayout);

    // 在线用户列表
    onlineUsersList = new QListWidget(this);
    onlineUsersList->setMaximumWidth(200);
    QHBoxLayout *splitLayout = new QHBoxLayout();
    splitLayout->addWidget(chatHistory, 3);
    splitLayout->addWidget(onlineUsersList, 1);
    mainLayout->insertLayout(0, splitLayout);

    // 状态栏
    statusLabel = new QLabel(this);
    mainLayout->addWidget(statusLabel);

    // 绑定回车键发送
    connect(messageInput, &QLineEdit::returnPressed, this, &ChatWindow::onSendMessage);
    connect(sendButton, &QPushButton::clicked, this, &ChatWindow::onSendMessage);
}

void ChatWindow::setupConnections() {
    // 已在构造函数中连接信号槽
}

void ChatWindow::onSendMessage() {
    QString message = messageInput->text().trimmed();
    if (message.isEmpty()) return;

    QString fullMessage = QString("[%1]: %2").arg(username).arg(message);
    chatHistory->append(fullMessage);
    messageInput->clear();

    networkManager->sendMessageToAllPeers(message);
}

void ChatWindow::onMessageReceived(const QString &message) {
    chatHistory->append(message);
}

void ChatWindow::onPeerDiscovered(const QString &ip, const QString &username) {
    QString itemText = QString("%1 (%2)").arg(username).arg(ip);
    for (int i = 0; i < onlineUsersList->count(); ++i) {
        if (onlineUsersList->item(i)->text() == itemText) {
            return; // 已存在
        }
    }
    onlineUsersList->addItem(itemText);
}