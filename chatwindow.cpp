#include "chatwindow.h"
#include <QApplication>
#include <QMessageBox>
#include <QToolButton>
#include <QMenu>
#include <QGridLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QAction>
#include <QWidgetAction>
#include <QFont>

ChatWindow::ChatWindow(QWidget *parent) : QWidget(parent) {
    // 初始化表情映射
    initEmojiMap();

    setupUI();
    setupConnections();
    createEmojiMenu();

    bool ok;
    username = QInputDialog::getText(this, "用户名", "请输入您的用户名:", QLineEdit::Normal, "", &ok);
    if (!ok || username.isEmpty()) {
        username = "匿名用户";
    }

    networkManager = new NetworkManager(this, username);
    connect(networkManager, &NetworkManager::messageReceived, this, &ChatWindow::onMessageReceived);
    connect(networkManager, &NetworkManager::peerDiscovered, this, &ChatWindow::onPeerDiscovered);

    statusLabel->setText(QString("就绪 - 用户名: %1 - 点击😊按钮发送表情").arg(username));
}

ChatWindow::~ChatWindow() {
    delete networkManager;
}

void ChatWindow::initEmojiMap() {
    // 初始化表情映射表
    emojiMap = {
        // 笑脸和情感
        {":)", "😊"},
        {":D", "😄"},
        {":(", "😞"},
        {";)", "😉"},
        {":P", "😛"},
        {":O", "😮"},
        {":*", "😘"},
        {":/", "😕"},
        {"B)", "😎"},
        {"o.O", "😳"},
        {"O_o", "😲"},
        {"3:)", "😈"},
        {":|", "😐"},
        {"*_*", "😍"},
        {"^^", "😊"},
        {"^_^", "😊"},
        {"-_-", "😑"},

        // 爱心
        {"<3", "❤️"},
        {"</3", "💔"},

        // 其他
        {"XD", "😆"},
        {"T_T", "😭"},
        {"-.-", "😒"},
        {":'>", "😊"},
        {"-.-'", "😅"},
        {":')", "😂"},

        // 手势
        {":+1:", "👍"},
        {":-1:", "👎"},
        {":ok:", "👌"},

        // 物品
        {":coffee:", "☕"},
        {":pizza:", "🍕"},
        {":beer:", "🍺"},
        {":cake:", "🎂"},
        {":gift:", "🎁"},
        {":star:", "⭐"},
        {":fire:", "🔥"}
    };

    // 常用表情列表（用于表情按钮菜单）
    commonEmojis = {
        "😊", "😄", "😂", "😍", "😘",
        "😉", "😎", "🥰", "😭", "😢",
        "😤", "😡", "🤬", "😱", "😨",
        "🤢", "🤮", "😴", "🤔", "🤫",
        "👍", "👎", "👏", "🙏", "🤝",
        "❤️", "💕", "💖", "💔", "💯",
        "🔥", "⭐", "🌟", "🎉", "🎊",
        "🎁", "🎂", "🍕", "🍺", "☕",
        "🐶", "🐱", "🐭", "🐹", "🐰",
        "🦊", "🐻", "🐼", "🐨", "🐯",
        "🦁", "🐮", "🐷", "🐸", "🐵"
    };
}

void ChatWindow::setupUI() {
    setWindowTitle("P2P 聊天室 - 支持表情包");
    resize(900, 700);

    mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(5);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // 聊天历史区域
    chatHistory = new QTextEdit(this);
    chatHistory->setReadOnly(true);
    chatHistory->setFont(QFont("Microsoft YaHei", 11));
    chatHistory->setStyleSheet("QTextEdit { background-color: #f9f9f9; border: 1px solid #ccc; border-radius: 5px; padding: 10px; }");
    mainLayout->addWidget(chatHistory, 4);

    // 输入区域
    QHBoxLayout *inputLayout = new QHBoxLayout();
    inputLayout->setSpacing(5);

    // 表情按钮
    emojiButton = new QToolButton(this);
    emojiButton->setText("😊");
    emojiButton->setToolTip("选择表情包");
    emojiButton->setFixedSize(40, 40);
    emojiButton->setStyleSheet("QToolButton { font-size: 20px; border: 1px solid #ccc; border-radius: 5px; background-color: #f0f0f0; }"
                               "QToolButton:hover { background-color: #e0e0e0; }");
    emojiButton->setPopupMode(QToolButton::InstantPopup);

    // 消息输入框
    messageInput = new QLineEdit(this);
    messageInput->setPlaceholderText("输入消息... (支持表情代码如 :) :D <3 等，点击😊按钮选择表情)");
    messageInput->setStyleSheet("QLineEdit { padding: 8px; border: 1px solid #ccc; border-radius: 5px; font-size: 14px; }");

    // 发送按钮
    sendButton = new QPushButton("发送", this);
    sendButton->setFixedWidth(80);
    sendButton->setStyleSheet("QPushButton { padding: 8px; background-color: #4CAF50; color: white; border: none; border-radius: 5px; font-weight: bold; }"
                             "QPushButton:hover { background-color: #45a049; }"
                             "QPushButton:pressed { background-color: #3d8b40; }");

    inputLayout->addWidget(emojiButton);
    inputLayout->addWidget(messageInput, 1);
    inputLayout->addWidget(sendButton);
    mainLayout->addLayout(inputLayout);

    // 分割布局：聊天历史 + 在线用户列表
    QHBoxLayout *splitLayout = new QHBoxLayout();
    splitLayout->setSpacing(10);

    // 在线用户列表容器
    QWidget *userListWidget = new QWidget(this);
    QVBoxLayout *userListLayout = new QVBoxLayout(userListWidget);
    userListLayout->setContentsMargins(5, 5, 5, 5);

    QLabel *userListTitle = new QLabel("👥 在线用户", this);
    userListTitle->setStyleSheet("font-weight: bold; font-size: 14px; padding: 5px;");
    userListLayout->addWidget(userListTitle);

    onlineUsersList = new QListWidget(this);
    onlineUsersList->setStyleSheet("QListWidget { border: 1px solid #ccc; border-radius: 5px; background-color: white; }"
                                  "QListWidget::item { padding: 8px; border-bottom: 1px solid #eee; }"
                                  "QListWidget::item:hover { background-color: #f0f0f0; }");
    userListLayout->addWidget(onlineUsersList);

    // 移除之前的聊天历史添加，重新添加
    mainLayout->removeWidget(chatHistory);
    splitLayout->addWidget(chatHistory, 3);
    splitLayout->addWidget(userListWidget, 1);
    mainLayout->insertLayout(0, splitLayout, 4);

    // 状态栏
    statusLabel = new QLabel(this);
    statusLabel->setStyleSheet("padding: 8px; background-color: #e8f5e9; border-top: 1px solid #c8e6c9; border-radius: 3px; font-size: 12px; color: #2e7d32;");
    mainLayout->addWidget(statusLabel);

    // 绑定回车键发送
    connect(messageInput, &QLineEdit::returnPressed, this, &ChatWindow::onSendMessage);
    connect(sendButton, &QPushButton::clicked, this, &ChatWindow::onSendMessage);

    // 欢迎消息
    chatHistory->append("💬 欢迎使用P2P聊天室！");
    chatHistory->append("✨ 支持表情包功能：");
    chatHistory->append("   1. 点击😊按钮选择表情");
    chatHistory->append("   2. 输入表情代码如 :) → 😊, :D → 😄, <3 → ❤️");
    chatHistory->append("   3. 按回车或点击发送按钮发送消息");
    chatHistory->append("");
}

void ChatWindow::createEmojiMenu() {
    emojiMenu = new QMenu(this);
    emojiMenu->setStyleSheet("QMenu { border: 1px solid #ccc; border-radius: 5px; background-color: white; }"
                           "QMenu::item { padding: 5px 20px 5px 5px; }"
                           "QMenu::item:selected { background-color: #e0e0e0; }");

    // 创建表情选择区域
    QWidget *emojiWidget = new QWidget(this);
    QGridLayout *gridLayout = new QGridLayout(emojiWidget);
    gridLayout->setSpacing(2);
    gridLayout->setContentsMargins(5, 5, 5, 5);

    // 添加常用表情
    int columns = 8;
    int rows = (commonEmojis.size() + columns - 1) / columns;

    for (int i = 0; i < commonEmojis.size(); i++) {
        QPushButton *emojiBtn = new QPushButton(commonEmojis[i]);
        emojiBtn->setFixedSize(35, 35);
        emojiBtn->setFont(QFont("Segoe UI Emoji", 16));
        emojiBtn->setStyleSheet("QPushButton { border: none; background-color: transparent; }"
                               "QPushButton:hover { background-color: #f0f0f0; border-radius: 3px; }"
                               "QPushButton:pressed { background-color: #e0e0e0; }");

        QString emoji = commonEmojis[i];
        connect(emojiBtn, &QPushButton::clicked, this, [this, emoji]() {
            insertEmoji(emoji);
            emojiMenu->hide();
        });

        gridLayout->addWidget(emojiBtn, i / columns, i % columns);
    }

    // 添加表情代码提示区域
    QWidget *emojiCodeWidget = new QWidget(this);
    QVBoxLayout *codeLayout = new QVBoxLayout(emojiCodeWidget);
    codeLayout->setContentsMargins(10, 10, 10, 10);

    QLabel *codeTitle = new QLabel("📝 表情代码表：", this);
    codeTitle->setStyleSheet("font-weight: bold; font-size: 12px;");
    codeLayout->addWidget(codeTitle);

    // 显示部分常用表情代码
    QStringList commonCodes = {":) → 😊", ":D → 😄", ":P → 😛", ";-) → 😉",
                              "<3 → ❤️", ":/ → 😕", "B) → 😎", "XD → 😆",
                              "T_T → 😭", ":* → 😘", "o.O → 😳", ":| → 😐"};

    for (const QString &code : commonCodes) {
        QLabel *codeLabel = new QLabel(code, this);
        codeLabel->setStyleSheet("font-size: 11px; padding: 2px;");
        codeLayout->addWidget(codeLabel);
    }

    // 创建滚动区域
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidget(emojiWidget);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setFixedHeight(200);
    scrollArea->setStyleSheet("QScrollArea { border: none; }");

    // 主布局
    QVBoxLayout *mainEmojiLayout = new QVBoxLayout();
    mainEmojiLayout->addWidget(scrollArea);
    mainEmojiLayout->addWidget(emojiCodeWidget);

    QWidget *containerWidget = new QWidget(this);
    containerWidget->setLayout(mainEmojiLayout);
    containerWidget->setFixedWidth(300);

    QWidgetAction *widgetAction = new QWidgetAction(emojiMenu);
    widgetAction->setDefaultWidget(containerWidget);
    emojiMenu->addAction(widgetAction);

    emojiButton->setMenu(emojiMenu);
}

void ChatWindow::setupConnections() {
    // 已在构造函数中连接信号槽
}

QString ChatWindow::processMessageWithEmojis(const QString &message) {
    QString result = message;

    // 先处理特殊的长代码
    result.replace(":coffee:", "☕");
    result.replace(":pizza:", "🍕");
    result.replace(":beer:", "🍺");
    result.replace(":cake:", "🎂");
    result.replace(":gift:", "🎁");
    result.replace(":star:", "⭐");
    result.replace(":fire:", "🔥");
    result.replace(":+1:", "👍");
    result.replace(":-1:", "👎");
    result.replace(":ok:", "👌");

    // 然后处理短代码
    for (auto it = emojiMap.begin(); it != emojiMap.end(); ++it) {
        // 只替换长度为2-4的代码，避免替换文本中的部分内容
        if (it.key().length() >= 2 && it.key().length() <= 4) {
            result.replace(it.key(), it.value());
        }
    }

    return result;
}

void ChatWindow::onSendMessage() {
    QString message = messageInput->text().trimmed();
    if (message.isEmpty()) return;

    // 处理表情代码
    QString processedMessage = processMessageWithEmojis(message);

    // 显示在聊天历史中
    QTextCursor cursor = chatHistory->textCursor();
    cursor.movePosition(QTextCursor::End);

    // 添加时间戳
    QString timestamp = QTime::currentTime().toString("hh:mm");
    QString fullMessage = QString("<div style='margin: 5px 0;'>"
                                 "<span style='color: #666; font-size: 11px;'>[%1]</span> "
                                 "<span style='color: #2196F3; font-weight: bold;'>%2:</span> "
                                 "<span style='font-size: 14px;'>%3</span>"
                                 "</div>")
                                 .arg(timestamp)
                                 .arg(username)
                                 .arg(processedMessage.toHtmlEscaped().replace("\n", "<br>"));

    chatHistory->append(fullMessage);
    chatHistory->moveCursor(QTextCursor::End);

    messageInput->clear();

    // 发送原始消息（包含表情代码），让接收方也进行转换
    networkManager->sendMessageToAllPeers(message);
}

void ChatWindow::onMessageReceived(const QString &message) {
    // 收到的消息可能包含表情代码，需要转换
    QString processedMessage = processMessageWithEmojis(message);

    // 提取用户名和消息内容
    QString displayMessage;
    if (message.startsWith("[") && message.contains("]: ")) {
        int bracketEnd = message.indexOf("]: ");
        QString usernamePart = message.mid(0, bracketEnd + 2);
        QString messagePart = message.mid(bracketEnd + 3);

        QString processedContent = processMessageWithEmojis(messagePart);
        displayMessage = usernamePart + processedContent;
    } else {
        displayMessage = processedMessage;
    }

    // 添加时间戳
    QString timestamp = QTime::currentTime().toString("hh:mm");
    QString fullMessage = QString("<div style='margin: 5px 0;'>"
                                 "<span style='color: #666; font-size: 11px;'>[%1]</span> "
                                 "%2"
                                 "</div>")
                                 .arg(timestamp)
                                 .arg(displayMessage.toHtmlEscaped().replace("\n", "<br>"));

    chatHistory->append(fullMessage);
    chatHistory->moveCursor(QTextCursor::End);
}

void ChatWindow::onPeerDiscovered(const QString &ip, const QString &username) {
    QString itemText = QString("👤 %1\n   📡 %2").arg(username).arg(ip);
    for (int i = 0; i < onlineUsersList->count(); ++i) {
        QListWidgetItem *item = onlineUsersList->item(i);
        if (item->text().contains(ip)) {
            // 更新现有用户
            item->setText(itemText);
            item->setForeground(QColor("#2e7d32")); // 绿色表示在线
            return;
        }
    }

    // 添加新用户
    QListWidgetItem *item = new QListWidgetItem(itemText, onlineUsersList);
    item->setForeground(QColor("#2e7d32")); // 绿色
    item->setFont(QFont("Microsoft YaHei", 10));
    onlineUsersList->addItem(item);

    // 显示通知
    chatHistory->append(QString("<div style='color: #666; font-size: 12px;'>📢 %1 加入了聊天室</div>").arg(username));
}

void ChatWindow::insertEmoji(const QString &emoji) {
    messageInput->insert(emoji);
    messageInput->setFocus();
}