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
#include <QFileDialog>
#include <QStandardPaths>
#include <QDir>
#include <QPixmap>
#include <QBuffer>
#include <QPainter>
#include <QTime>
#include <QPainterPath>

ChatWindow::ChatWindow(const QString &username, const QString &avatarPath, QWidget *parent)
    : QWidget(parent), username(username), avatarPath(avatarPath)
{
    // 如果传入了空用户名，使用默认用户名
    if (this->username.isEmpty()) {
        this->username = "匿名用户";
    }

    // 初始化表情映射
    initEmojiMap();

    setupUI();
    setupConnections();
    createEmojiMenu();

    // 加载用户头像
    loadUserAvatar();

    networkManager = new NetworkManager(this, this->username);
    connect(networkManager, &NetworkManager::messageReceived, this, &ChatWindow::onMessageReceived);
    connect(networkManager, &NetworkManager::peerDiscovered, this, &ChatWindow::onPeerDiscovered);
    // 连接头像按钮点击信号
    connect(avatarButton, &QPushButton::clicked, this, &ChatWindow::onAvatarButtonClicked);

    statusLabel->setText(QString("就绪 - 用户名: %1 - 点击😊按钮发送表情").arg(this->username));
}


ChatWindow::~ChatWindow() {
    delete networkManager;
}


void ChatWindow::initEmojiMap() {
    // 初始化表情映射表
    emojiMap = {
        {":)", "😊"}, {":-)", "😊"}, {":D", "😄"}, {":-D", "😄"},
        {":(", "😞"}, {":-(", "😞"}, {":'(", "😢"}, {":O", "😲"},
        {":-O", "😲"}, {":P", "😛"}, {":-P", "😛"}, {";)", "😉"},
        {";-)", "😉"}, {"<3", "❤️"}, {"</3", "💔"}, {":*", "😘"},
        {":-*", "😘"}, {":|", "😐"}, {":-|", "😐"}, {"XD", "😆"},
        {"xD", "😆"}, {"xDD", "😂"}, {"^^", "😊"}, {">:(", "😠"},
        {">:-(", "😠"}, {"O:)", "😇"}, {"O:-)", "😇"}, {"3:)", "😈"},
        {"3:-)", "😈"}, {"o.O", "😳"}, {"O.o", "😳"}, {":/", "😕"},
        {":-/", "😕"}, {":\\", "😕"}, {":-\\", "😕"}, {":$", "😳"},
        {":-$", "😳"}, {"B)", "😎"}, {"B-)", "😎"}, {"8)", "😎"},
        {"8-)", "😎"}, {"':(", "😥"}, {"':-)", "😥"}, {"'):", "😥"},
        {"'-):", "😥"}, {"</3", "💔"}, {"(y)", "👍"}, {"(n)", "👎"},
        {"(Y)", "👍"}, {"(N)", "👎"}, {"(ok)", "👌"}, {"(OK)", "👌"}
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

    // 设置窗口图标
    QIcon icon(":/icons/chat.png");
    if (!icon.isNull()) {
        setWindowIcon(icon);
    }

    mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(5);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // 分割布局：聊天历史 + 在线用户列表
    QHBoxLayout *splitLayout = new QHBoxLayout();
    splitLayout->setSpacing(10);

    // 聊天历史区域
    chatHistory = new QTextEdit(this);
    chatHistory->setReadOnly(true);
    chatHistory->setFont(QFont("Microsoft YaHei", 11));
    chatHistory->setStyleSheet("QTextEdit { "
                              "background-color: #f9f9f9; "
                              "border: 1px solid #ccc; "
                              "border-radius: 8px; "
                              "padding: 10px; "
                              "}");

    // 在线用户列表容器
    QWidget *userListWidget = new QWidget(this);
    userListWidget->setMinimumWidth(200);
    userListWidget->setMaximumWidth(250);
    QVBoxLayout *userListLayout = new QVBoxLayout(userListWidget);
    userListLayout->setContentsMargins(5, 5, 5, 5);
    userListLayout->setSpacing(5);

    QLabel *userListTitle = new QLabel("👥 在线用户", this);
    userListTitle->setStyleSheet("font-weight: bold; font-size: 14px; padding: 8px; color: #333;");
    userListLayout->addWidget(userListTitle);

    onlineUsersList = new QListWidget(this);
    onlineUsersList->setStyleSheet("QListWidget { "
                                  "border: 1px solid #ccc; "
                                  "border-radius: 8px; "
                                  "background-color: white; "
                                  "}"
                                  "QListWidget::item { "
                                  "padding: 8px; "
                                  "border-bottom: 1px solid #eee; "
                                  "}"
                                  "QListWidget::item:hover { "
                                  "background-color: #f5f5f5; "
                                  "}");
    userListLayout->addWidget(onlineUsersList);

    splitLayout->addWidget(chatHistory, 3);
    splitLayout->addWidget(userListWidget, 1);
    mainLayout->addLayout(splitLayout, 4);

    // 输入区域
    QHBoxLayout *inputLayout = new QHBoxLayout();
    inputLayout->setSpacing(8);

    // 头像按钮
    avatarButton = new QPushButton(this);
    avatarButton->setFixedSize(50, 50);
    avatarButton->setStyleSheet("QPushButton { "
                               "border: 2px solid #4CAF50; "
                               "border-radius: 25px; "
                               "background-color: #f0f0f0; "
                               "font-size: 20px; "
                               "padding: 0px; "
                               "}"
                               "QPushButton:hover { "
                               "background-color: #e8f5e9; "
                               "border-color: #45a049; "
                               "}"
                               "QPushButton:pressed { "
                               "background-color: #c8e6c9; "
                               "}");
    avatarButton->setText("👤");
    avatarButton->setToolTip("设置头像\n当前只能设置一次");

    // 文件发送按钮
    fileButton = new QPushButton("📁", this);
    fileButton->setFixedSize(50, 50);
    fileButton->setStyleSheet("QPushButton { "
                             "font-size: 22px; "
                             "border: 1px solid #ccc; "
                             "border-radius: 8px; "
                             "background-color: #f0f0f0; "
                             "}"
                             "QPushButton:hover { "
                             "background-color: #e0e0e0; "
                             "border-color: #4CAF50; "
                             "}"
                             "QPushButton:pressed { "
                             "background-color: #d0d0d0; "
                             "}");
    fileButton->setToolTip("发送文件");

    // 表情按钮
    emojiButton = new QToolButton(this);
    emojiButton->setText("😊");
    emojiButton->setToolTip("选择表情包");
    emojiButton->setFixedSize(50, 50);
    emojiButton->setStyleSheet("QToolButton { "
                              "font-size: 22px; "
                              "border: 1px solid #ccc; "
                              "border-radius: 8px; "
                              "background-color: #f0f0f0; "
                              "}"
                              "QToolButton:hover { "
                              "background-color: #fff9c4; "
                              "border-color: #4CAF50; "
                              "}"
                              "QToolButton:pressed { "
                              "background-color: #fff59d; "
                              "}");

    // 消息输入框
    messageInput = new QLineEdit(this);
    messageInput->setPlaceholderText("输入消息... (支持表情代码如 :) :D <3 等，点击😊按钮选择表情)");
    messageInput->setStyleSheet("QLineEdit { "
                               "padding: 12px; "
                               "border: 1px solid #ccc; "
                               "border-radius: 8px; "
                               "font-size: 14px; "
                               "background-color: white; "
                               "}"
                               "QLineEdit:focus { "
                               "border-color: #4CAF50; "
                               "border-width: 2px; "
                               "}");

    // 发送按钮
    sendButton = new QPushButton("发送", this);
    sendButton->setFixedWidth(100);
    sendButton->setFixedHeight(50);
    sendButton->setStyleSheet("QPushButton { "
                             "padding: 12px; "
                             "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
                             "stop:0 #4CAF50, stop:1 #45a049); "
                             "color: white; "
                             "border: none; "
                             "border-radius: 8px; "
                             "font-weight: bold; "
                             "font-size: 14px; "
                             "}"
                             "QPushButton:hover { "
                             "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
                             "stop:0 #45a049, stop:1 #3d8b40); "
                             "}"
                             "QPushButton:pressed { "
                             "background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
                             "stop:0 #3d8b40, stop:1 #2e7d32); "
                             "}");

    inputLayout->addWidget(avatarButton);
    inputLayout->addWidget(fileButton);
    inputLayout->addWidget(emojiButton);
    inputLayout->addWidget(messageInput, 1);
    inputLayout->addWidget(sendButton);
    mainLayout->addLayout(inputLayout);

    // 状态栏
    statusLabel = new QLabel(this);
    statusLabel->setStyleSheet("padding: 8px 12px; "
                              "background: linear-gradient(to right, #e8f5e9, #c8e6c9); "
                              "border-top: 1px solid #a5d6a7; "
                              "border-radius: 6px; "
                              "font-size: 12px; "
                              "color: #2e7d32;");
    mainLayout->addWidget(statusLabel);

    // 绑定回车键发送
    connect(messageInput, &QLineEdit::returnPressed, this, &ChatWindow::onSendMessage);
    connect(sendButton, &QPushButton::clicked, this, &ChatWindow::onSendMessage);
    connect(fileButton, &QPushButton::clicked, this, &ChatWindow::onSendFile);

    // 显示欢迎消息
    QTimer::singleShot(100, this, [this]() {
        QString welcomeMessage = QString("<div style='text-align: center; color: #666; padding: 10px;'>"
                                        "欢迎 <b>%1</b> 加入P2P聊天室！"
                                        "<br><small>现在可以开始与在线用户聊天了</small>"
                                        "</div>").arg(username);
        chatHistory->append(welcomeMessage);
    });
}

void ChatWindow::createEmojiMenu() {
    emojiMenu = new QMenu(this);
    emojiMenu->setStyleSheet("QMenu { "
                            "border: 1px solid #ccc; "
                            "border-radius: 8px; "
                            "background-color: white; "
                            "box-shadow: 0 4px 12px rgba(0,0,0,0.1); "
                            "}"
                            "QMenu::item { "
                            "padding: 8px 16px; "
                            "border-radius: 4px; "
                            "}"
                            "QMenu::item:selected { "
                            "background-color: #e8f5e9; "
                            "}");

    // 创建表情选择区域
    QWidget *emojiWidget = new QWidget(this);
    QGridLayout *gridLayout = new QGridLayout(emojiWidget);
    gridLayout->setSpacing(4);
    gridLayout->setContentsMargins(8, 8, 8, 8);

    // 添加常用表情
    int columns = 10;
    int rows = (commonEmojis.size() + columns - 1) / columns;

    for (int i = 0; i < commonEmojis.size(); i++) {
        QPushButton *emojiBtn = new QPushButton(commonEmojis[i]);
        emojiBtn->setFixedSize(36, 36);
        emojiBtn->setFont(QFont("Segoe UI Emoji", 18));
        emojiBtn->setStyleSheet("QPushButton { "
                               "border: none; "
                               "background-color: transparent; "
                               "border-radius: 6px; "
                               "}"
                               "QPushButton:hover { "
                               "background-color: #f5f5f5; "
                               "border: 1px solid #e0e0e0; "
                               "}"
                               "QPushButton:pressed { "
                               "background-color: #e0e0e0; "
                               "}");

        QString emoji = commonEmojis[i];
        connect(emojiBtn, &QPushButton::clicked, this, [this, emoji]() {
            insertEmoji(emoji);
            emojiMenu->close();
        });

        gridLayout->addWidget(emojiBtn, i / columns, i % columns);
    }

    // 创建滚动区域
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidget(emojiWidget);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setFixedHeight(220);
    scrollArea->setStyleSheet("QScrollArea { border: none; background: transparent; }"
                             "QScrollBar:vertical { width: 12px; background: #f0f0f0; border-radius: 6px; }"
                             "QScrollBar::handle:vertical { background: #c0c0c0; border-radius: 6px; min-height: 20px; }"
                             "QScrollBar::handle:vertical:hover { background: #a0a0a0; }");

    // 主布局
    QVBoxLayout *mainEmojiLayout = new QVBoxLayout();
    mainEmojiLayout->addWidget(scrollArea);

    // 添加表情代码提示
    QLabel *hintLabel = new QLabel("提示：在输入框中直接输入表情代码，如 :) :D <3");
    hintLabel->setStyleSheet("padding: 8px; color: #666; font-size: 11px; background: #f9f9f9; border-radius: 4px;");
    mainEmojiLayout->addWidget(hintLabel);

    QWidget *containerWidget = new QWidget(this);
    containerWidget->setLayout(mainEmojiLayout);
    containerWidget->setFixedWidth(380);

    QWidgetAction *widgetAction = new QWidgetAction(emojiMenu);
    widgetAction->setDefaultWidget(containerWidget);
    emojiMenu->addAction(widgetAction);

    emojiButton->setMenu(emojiMenu);
    emojiButton->setPopupMode(QToolButton::InstantPopup);
}

void ChatWindow::setupConnections() {
    // 已在构造函数中连接信号槽
}

QString ChatWindow::processMessageWithEmojis(const QString &message) {
    QString result = message;

    // 先处理特殊的长代码
    QMap<QString, QString> specialEmojis = {
        {":coffee:", "☕"}, {":pizza:", "🍕"}, {":beer:", "🍺"},
        {":cake:", "🎂"}, {":gift:", "🎁"}, {":star:", "⭐"},
        {":fire:", "🔥"}, {":+1:", "👍"}, {":-1:", "👎"},
        {":ok:", "👌"}, {":100:", "💯"}, {":heart:", "❤️"},
        {":thumbsup:", "👍"}, {":thumbsdown:", "👎"}, {":clap:", "👏"},
        {":pray:", "🙏"}, {":handshake:", "🤝"}
    };

    for (auto it = specialEmojis.begin(); it != specialEmojis.end(); ++it) {
        result.replace(it.key(), it.value());
    }

    // 然后处理短代码
    for (auto it = emojiMap.begin(); it != emojiMap.end(); ++it) {
        // 使用正则表达式确保只替换完整的代码
        // 注意：这里可能需要包含头文件 #include <QRegularExpression>
        QString pattern = "\\b" + QRegularExpression::escape(it.key()) + "\\b";
        QRegularExpression rx(pattern);
        result.replace(rx, it.value());
    }

    return result;
}

void ChatWindow::onSendMessage() {
    QString message = messageInput->text().trimmed();
    if (message.isEmpty()) return;

    // 处理表情代码
    QString processedMessage = processMessageWithEmojis(message);

    // 将头像信息编码到消息中
    QString avatarData = "";
    if (!avatarPath.isEmpty()) {
        QPixmap avatarPixmap(avatarPath);
        if (!avatarPixmap.isNull()) {
            avatarPixmap = cropToSquare(avatarPixmap);
            avatarPixmap = avatarPixmap.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            QByteArray byteArray;
            QBuffer buffer(&byteArray);
            buffer.open(QIODevice::WriteOnly);
            avatarPixmap.save(&buffer, "PNG");
            avatarData = QString::fromLatin1(byteArray.toBase64().data());
        }
    }

    // 构造带头像信息的消息
    QString fullMessage = QString("[%1]: %2").arg(username).arg(message);
    if (!avatarData.isEmpty()) {
        fullMessage += QString("|AVATAR:%1").arg(avatarData);
    }

    // 显示在聊天历史中（带头像）
    QTextCursor cursor = chatHistory->textCursor();
    cursor.movePosition(QTextCursor::End);

    // 添加时间戳
    QString timestamp = QTime::currentTime().toString("hh:mm:ss");

    // 获取当前用户头像
    QString avatarHtml = "";
    if (!avatarData.isEmpty()) {
        avatarHtml = QString("<img src='data:image/png;base64,%1' width='32' height='32' "
                            "style='vertical-align: middle; margin-right: 8px; border-radius: 16px; "
                            "border: 1px solid #4CAF50; box-shadow: 0 2px 4px rgba(0,0,0,0.1);' />").arg(avatarData);
    } else {
        // 默认头像
        avatarHtml = "<div style='width: 32px; height: 32px; background: linear-gradient(135deg, #4CAF50, #45a049); "
                    "border-radius: 16px; margin-right: 8px; display: flex; align-items: center; "
                    "justify-content: center; font-size: 16px; color: white; box-shadow: 0 2px 4px rgba(0,0,0,0.1);'>👤</div>";
    }

    QString displayMessage = QString("<div style='margin: 8px 0; display: flex; align-items: flex-start;'>"
                                   "%1"
                                   "<div style='flex: 1;'>"
                                   "<div style='display: flex; align-items: center; margin-bottom: 4px;'>"
                                   "<span style='color: #2196F3; font-weight: bold; font-size: 14px;'>%2</span>"
                                   "<span style='color: #999; font-size: 11px; margin-left: 8px;'>%3</span>"
                                   "</div>"
                                   "<div style='background: linear-gradient(135deg, #e3f2fd, #bbdefb); "
                                   "padding: 10px 12px; border-radius: 12px; border-top-left-radius: 4px; "
                                   "font-size: 14px; line-height: 1.4; color: #333; "
                                   "box-shadow: 0 2px 4px rgba(0,0,0,0.05);'>%4</div>"
                                   "</div>"
                                   "</div>")
                                   .arg(avatarHtml)
                                   .arg(username)
                                   .arg(timestamp)
                                   .arg(processedMessage.toHtmlEscaped().replace("\n", "<br>"));

    chatHistory->append(displayMessage);
    chatHistory->moveCursor(QTextCursor::End);

    messageInput->clear();

    // 发送带头像信息的消息
    networkManager->sendMessageToAllPeers(fullMessage);
}

void ChatWindow::onMessageReceived(const QString &message) {
    // 检查是否是文件消息
    if (message.contains("[FILE]") && message.contains("[FILENAME]")) {
        handleFileMessage(message);
        return;
    }

    // 处理普通文本消息
    QString displayMessage;
    QString senderUsername = "未知用户";
    QString avatarData = "";

    QString actualMessage = message;

    // 检查是否有头像数据
    if (message.contains("|AVATAR:")) {
        int avatarPos = message.lastIndexOf("|AVATAR:");
        actualMessage = message.left(avatarPos);
        avatarData = message.mid(avatarPos + 8); // 跳过 "|AVATAR:" 前缀
    }

    if (actualMessage.startsWith("[") && actualMessage.contains("]: ")) {
        int bracketEnd = actualMessage.indexOf("]: ");
        QString usernamePart = actualMessage.mid(1, bracketEnd - 1);  // 提取用户名
        senderUsername = usernamePart;
        QString messagePart = actualMessage.mid(bracketEnd + 3);

        // 处理表情代码
        QString processedContent = processMessageWithEmojis(messagePart);
        displayMessage = processedContent;
    } else {
        displayMessage = processMessageWithEmojis(actualMessage);
    }

    // 如果有头像数据，存储到用户头像缓存中
    if (!avatarData.isEmpty() && !senderUsername.isEmpty()) {
        QPixmap avatarPixmap;
        QByteArray avatarBytes = QByteArray::fromBase64(avatarData.toLatin1());
        avatarPixmap.loadFromData(avatarBytes);
        if (!avatarPixmap.isNull()) {
            userAvatars[senderUsername] = avatarPixmap;

            // 更新在线用户列表中的头像
            updateOnlineUserAvatar(senderUsername, avatarPixmap);
        }
    }

    // 添加时间戳
    QString timestamp = QTime::currentTime().toString("hh:mm:ss");

    // 获取发送者头像
    QString avatarHtml = "";
    if (!avatarData.isEmpty()) {
        avatarHtml = QString("<img src='data:image/png;base64,%1' width='32' height='32' "
                            "style='vertical-align: middle; margin-right: 8px; border-radius: 16px; "
                            "border: 1px solid #4CAF50; box-shadow: 0 2px 4px rgba(0,0,0,0.1);' />").arg(avatarData);
    } else {
        // 尝试从缓存获取头像
        QPixmap senderAvatar = getUserAvatar(senderUsername);
        if (!senderAvatar.isNull()) {
            senderAvatar = cropToSquare(senderAvatar);
            senderAvatar = senderAvatar.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            QByteArray byteArray;
            QBuffer buffer(&byteArray);
            buffer.open(QIODevice::WriteOnly);
            senderAvatar.save(&buffer, "PNG");
            QString base64Image = QString::fromLatin1(byteArray.toBase64().data());
            avatarHtml = QString("<img src='data:image/png;base64,%1' width='32' height='32' "
                                "style='vertical-align: middle; margin-right: 8px; border-radius: 16px; "
                                "border: 1px solid #4CAF50; box-shadow: 0 2px 4px rgba(0,0,0,0.1);' />").arg(base64Image);
        } else {
            // 默认头像
            avatarHtml = "<div style='width: 32px; height: 32px; background: linear-gradient(135deg, #4CAF50, #45a049); "
                        "border-radius: 16px; margin-right: 8px; display: flex; align-items: center; "
                        "justify-content: center; font-size: 16px; color: white; box-shadow: 0 2px 4px rgba(0,0,0,0.1);'>👤</div>";
        }
    }

    QString fullMessage = QString("<div style='margin: 8px 0; display: flex; align-items: flex-start;'>"
                                 "%1"
                                 "<div style='flex: 1;'>"
                                 "<div style='display: flex; align-items: center; margin-bottom: 4px;'>"
                                 "<span style='color: #4CAF50; font-weight: bold; font-size: 14px;'>%2</span>"
                                 "<span style='color: #999; font-size: 11px; margin-left: 8px;'>%3</span>"
                                 "</div>"
                                 "<div style='background: linear-gradient(135deg, #f1f8e9, #dcedc8); "
                                 "padding: 10px 12px; border-radius: 12px; border-top-left-radius: 4px; "
                                 "font-size: 14px; line-height: 1.4; color: #333; "
                                 "box-shadow: 0 2px 4px rgba(0,0,0,0.05);'>%4</div>"
                                 "</div>"
                                 "</div>")
                                 .arg(avatarHtml)
                                 .arg(senderUsername)
                                 .arg(timestamp)
                                 .arg(displayMessage.toHtmlEscaped().replace("\n", "<br>"));

    chatHistory->append(fullMessage);
    chatHistory->moveCursor(QTextCursor::End);
}

void ChatWindow::handleFileMessage(const QString &message) {
    // 解析文件消息
    int startBracket = message.indexOf('[');
    int endBracket = message.indexOf(']');
    QString senderUsername = message.mid(startBracket + 1, endBracket - startBracket - 1);

    int fileStart = message.indexOf("[FILE]") + 6;
    int filenameStart = message.indexOf("[FILENAME]");
    int extensionStart = message.indexOf("[FILEEXTENSION]");
    int filetypeStart = message.indexOf("[FILETYPE]");
    int filesizeStart = message.indexOf("[FILESIZE]");
    int filedataStart = message.indexOf("[FILEDATA]");
    int fileEnd = message.indexOf("[/FILE]");

    if (fileStart >= 6 && filenameStart > fileStart && extensionStart > filenameStart &&
        filetypeStart > extensionStart && filesizeStart > filetypeStart && filedataStart > filesizeStart && fileEnd > filedataStart) {

        QString fileName = message.mid(fileStart, filenameStart - fileStart);
        QString fileExtension = message.mid(filenameStart + 10, extensionStart - filenameStart - 10);
        QString fileType = message.mid(extensionStart + 15, filetypeStart - extensionStart - 15);
        QString fileSizeStr = message.mid(filetypeStart + 10, filesizeStart - filetypeStart - 10);

        // 提取文件数据部分
        QString filePart = message.mid(filedataStart + 10, fileEnd - filedataStart - 10);
        int thumbnailPos = filePart.indexOf("[THUMBNAIL]");
        QString fileDataBase64, thumbnailBase64;

        if (thumbnailPos != -1) {
            fileDataBase64 = filePart.left(thumbnailPos);
            thumbnailBase64 = filePart.mid(thumbnailPos + 11);
        } else {
            fileDataBase64 = filePart;
        }

        // 解码文件数据
        QByteArray fileData = QByteArray::fromBase64(fileDataBase64.toUtf8());
        qint64 fileSize = fileSizeStr.toLongLong();

        bool isImage = (fileType == "image");
        bool isVideo = (fileType == "video");

        // 添加时间戳
        QString timestamp = QTime::currentTime().toString("hh:mm:ss");

        // 获取发送者头像
        QString avatarHtml = "";
        QPixmap senderAvatar = getUserAvatar(senderUsername);
        if (!senderAvatar.isNull()) {
            senderAvatar = cropToSquare(senderAvatar);
            senderAvatar = senderAvatar.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            QByteArray byteArray;
            QBuffer buffer(&byteArray);
            buffer.open(QIODevice::WriteOnly);
            senderAvatar.save(&buffer, "PNG");
            QString base64Image = QString::fromLatin1(byteArray.toBase64().data());
            avatarHtml = QString("<img src='data:image/png;base64,%1' width='32' height='32' "
                                "style='vertical-align: middle; margin-right: 8px; border-radius: 16px; "
                                "border: 1px solid #4CAF50; box-shadow: 0 2px 4px rgba(0,0,0,0.1);' />").arg(base64Image);
        } else {
            avatarHtml = "<div style='width: 32px; height: 32px; background: linear-gradient(135deg, #4CAF50, #45a049); "
                        "border-radius: 16px; margin-right: 8px; display: flex; align-items: center; "
                        "justify-content: center; font-size: 16px; color: white; box-shadow: 0 2px 4px rgba(0,0,0,0.1);'>👤</div>";
        }

        // 根据文件类型显示不同内容
        QString fileMessage;
        if (isImage) {
            QString thumbnailHtml = "";
            if (!thumbnailBase64.isEmpty()) {
                thumbnailHtml = QString("<img src='data:image/jpeg;base64,%1' "
                                       "style='max-width: 120px; max-height: 120px; margin-top: 8px; "
                                       "border-radius: 8px; border: 1px solid #ddd; "
                                       "box-shadow: 0 2px 8px rgba(0,0,0,0.1); cursor: pointer;' "
                                       "onclick='this.style.maxWidth=\"none\"; this.style.maxHeight=\"none\"'/>")
                                       .arg(thumbnailBase64);
            }

            fileMessage = QString("<div style='margin: 12px 0; display: flex; align-items: flex-start;'>"
                                 "%1"
                                 "<div style='flex: 1;'>"
                                 "<div style='display: flex; align-items: center; margin-bottom: 4px;'>"
                                 "<span style='color: #4CAF50; font-weight: bold; font-size: 14px;'>%2</span>"
                                 "<span style='color: #999; font-size: 11px; margin-left: 8px;'>%3</span>"
                                 "</div>"
                                 "<div style='background: #f9f9f9; padding: 12px; border-radius: 12px; "
                                 "border: 1px dashed #4CAF50;'>"
                                 "<div style='color: #666; margin-bottom: 8px;'>"
                                 "📸 发送了图片：<b>%4</b> (%5 KB)"
                                 "</div>"
                                 "%6"
                                 "<br>"
                                 "<button onclick='saveReceivedFile(\"%2\", \"%4\")' "
                                 "style='margin-top: 8px; padding: 6px 12px; "
                                 "background: linear-gradient(135deg, #4CAF50, #45a049); "
                                 "color: white; border: none; border-radius: 6px; "
                                 "font-size: 12px; cursor: pointer;'>"
                                 "💾 保存图片"
                                 "</button>"
                                 "</div>"
                                 "</div>"
                                 "</div>")
                                 .arg(avatarHtml)
                                 .arg(senderUsername)
                                 .arg(timestamp)
                                 .arg(fileName)
                                 .arg(fileSize / 1024)
                                 .arg(thumbnailHtml);
        } else if (isVideo) {
            fileMessage = QString("<div style='margin: 12px 0; display: flex; align-items: flex-start;'>"
                                 "%1"
                                 "<div style='flex: 1;'>"
                                 "<div style='display: flex; align-items: center; margin-bottom: 4px;'>"
                                 "<span style='color: #4CAF50; font-weight: bold; font-size: 14px;'>%2</span>"
                                 "<span style='color: #999; font-size: 11px; margin-left: 8px;'>%3</span>"
                                 "</div>"
                                 "<div style='background: #f9f9f9; padding: 12px; border-radius: 12px; "
                                 "border: 1px dashed #4CAF50;'>"
                                 "<div style='color: #666; margin-bottom: 8px;'>"
                                 "🎬 发送了视频：<b>%4</b> (%5 KB)"
                                 "</div>"
                                 "<div style='width: 120px; height: 120px; "
                                 "background: linear-gradient(135deg, #333, #555); "
                                 "border-radius: 8px; margin-top: 8px; display: flex; "
                                 "align-items: center; justify-content: center; color: white; "
                                 "font-size: 24px;'>"
                                 "🎬"
                                 "</div>"
                                 "<br>"
                                 "<button onclick='saveReceivedFile(\"%2\", \"%4\")' "
                                 "style='margin-top: 8px; padding: 6px 12px; "
                                 "background: linear-gradient(135deg, #4CAF50, #45a049); "
                                 "color: white; border: none; border-radius: 6px; "
                                 "font-size: 12px; cursor: pointer;'>"
                                 "📥 保存视频"
                                 "</button>"
                                 "</div>"
                                 "</div>"
                                 "</div>")
                                 .arg(avatarHtml)
                                 .arg(senderUsername)
                                 .arg(timestamp)
                                 .arg(fileName)
                                 .arg(fileSize / 1024);
        } else {
            fileMessage = QString("<div style='margin: 12px 0; display: flex; align-items: flex-start;'>"
                                 "%1"
                                 "<div style='flex: 1;'>"
                                 "<div style='display: flex; align-items: center; margin-bottom: 4px;'>"
                                 "<span style='color: #4CAF50; font-weight: bold; font-size: 14px;'>%2</span>"
                                 "<span style='color: #999; font-size: 11px; margin-left: 8px;'>%3</span>"
                                 "</div>"
                                 "<div style='background: #f9f9f9; padding: 12px; border-radius: 12px; "
                                 "border: 1px dashed #4CAF50;'>"
                                 "<div style='color: #666; margin-bottom: 8px;'>"
                                 "📁 发送了文件：<b>%4</b> (%5 KB)"
                                 "</div>"
                                 "<div style='width: 120px; height: 120px; "
                                 "background: linear-gradient(135deg, #e0e0e0, #f0f0f0); "
                                 "border-radius: 8px; margin-top: 8px; display: flex; "
                                 "align-items: center; justify-content: center; color: #666; "
                                 "font-size: 32px;'>"
                                 "📁"
                                 "</div>"
                                 "<br>"
                                 "<button onclick='saveReceivedFile(\"%2\", \"%4\")' "
                                 "style='margin-top: 8px; padding: 6px 12px; "
                                 "background: linear-gradient(135deg, #4CAF50, #45a049); "
                                 "color: white; border: none; border-radius: 6px; "
                                 "font-size: 12px; cursor: pointer;'>"
                                 "💾 保存文件"
                                 "</button>"
                                 "</div>"
                                 "</div>"
                                 "</div>")
                                 .arg(avatarHtml)
                                 .arg(senderUsername)
                                 .arg(timestamp)
                                 .arg(fileName)
                                 .arg(fileSize / 1024);
        }

        chatHistory->append(fileMessage);
        chatHistory->moveCursor(QTextCursor::End);

        // 临时存储文件数据，等待用户保存
        QString fileKey = QString("%1_%2").arg(senderUsername).arg(fileName);
        receivedFiles[fileKey] = QString::fromLatin1(fileData.toBase64().data());
    }
}

void ChatWindow::onPeerDiscovered(const QString &ip, const QString &username) {
    // 创建自定义的列表项widget
    QListWidgetItem *existingItem = nullptr;
    int existingRow = -1;

    for (int i = 0; i < onlineUsersList->count(); ++i) {
        QListWidgetItem *item = onlineUsersList->item(i);
        if (item->text().contains(ip)) {
            existingItem = item;
            existingRow = i;
            break;
        }
    }

    QString itemText = QString("%1\n   📡 %2").arg(username).arg(ip);

    // 创建带头像的列表项
    if (existingItem) {
        // 更新现有用户
        existingItem->setText(itemText);
        existingItem->setForeground(QColor("#2e7d32"));

        // 检查是否有缓存的头像
        if (userAvatars.contains(username)) {
            QPixmap userAvatar = userAvatars[username];
            userAvatar = cropToSquare(userAvatar);
            userAvatar = userAvatar.scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            existingItem->setIcon(QIcon(userAvatar));
        }
        return;
    }

    // 添加新用户
    QListWidgetItem *item = new QListWidgetItem(itemText, onlineUsersList);
    item->setForeground(QColor("#2e7d32"));
    item->setFont(QFont("Microsoft YaHei", 10));

    // 设置用户头像（如果有）
    QPixmap userAvatar = getUserAvatar(username);
    if (!userAvatar.isNull()) {
        userAvatar = cropToSquare(userAvatar);
        userAvatar = userAvatar.scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        item->setIcon(QIcon(userAvatar));
    } else {
        // 创建默认头像
        QPixmap defaultAvatar(24, 24);
        defaultAvatar.fill(Qt::transparent);
        QPainter painter(&defaultAvatar);
        painter.setRenderHint(QPainter::Antialiasing);

        QPainterPath path;
        path.addEllipse(0, 0, 24, 24);
        painter.setClipPath(path);

        QLinearGradient gradient(0, 0, 24, 24);
        gradient.setColorAt(0, QColor(76, 175, 80));
        gradient.setColorAt(1, QColor(56, 142, 60));
        painter.setBrush(gradient);
        painter.drawEllipse(0, 0, 24, 24);

        painter.setClipping(false);
        painter.setPen(Qt::NoPen);
        painter.setBrush(Qt::white);
        painter.setFont(QFont("Segoe UI Emoji", 12));
        painter.drawText(defaultAvatar.rect(), Qt::AlignCenter, "👤");
        painter.end();

        item->setIcon(QIcon(defaultAvatar));
    }

    onlineUsersList->addItem(item);
}

void ChatWindow::insertEmoji(const QString &emoji) {
    messageInput->insert(emoji);
    messageInput->setFocus();
}

void ChatWindow::onAvatarButtonClicked() {
    // 如果已经设置了头像，则不允許再次設置
    if (!avatarPath.isEmpty()) {
        QMessageBox::information(this, "提示", "您已经设置了头像，无法再次修改！");
        return;
    }

    // 打开文件选择对话框
    QString fileName = QFileDialog::getOpenFileName(this,
                                                   tr("选择头像"),
                                                   "",
                                                   tr("图片文件 (*.png *.jpg *.bmp *.jpeg *.gif)"));

    if (!fileName.isEmpty()) {
        // 加载并裁剪头像
        QPixmap pixmap(fileName);
        if (!pixmap.isNull()) {
            pixmap = cropToSquare(pixmap);
            // 保存头像
            saveUserAvatar(fileName);

            // 显示头像
            QPixmap scaledPixmap = pixmap.scaled(46, 46, Qt::KeepAspectRatio, Qt::SmoothTransformation);

            // 创建圆形头像
            QPixmap circularPixmap(46, 46);
            circularPixmap.fill(Qt::transparent);
            QPainter painter(&circularPixmap);
            painter.setRenderHint(QPainter::Antialiasing);
            QPainterPath path;
            path.addEllipse(0, 0, 46, 46);
            painter.setClipPath(path);
            painter.drawPixmap(0, 0, scaledPixmap);
            painter.end();

            avatarButton->setIcon(QIcon(circularPixmap));
            avatarButton->setIconSize(QSize(46, 46));
            avatarButton->setText(""); // 清除文字
        }
    }
}

void ChatWindow::loadUserAvatar() {
    // 如果有从登录界面传入的头像路径，直接使用
    if (!avatarPath.isEmpty() && QFile::exists(avatarPath)) {
        QPixmap pixmap(avatarPath);
        if (!pixmap.isNull()) {
            pixmap = cropToSquare(pixmap);
            QPixmap scaledPixmap = pixmap.scaled(46, 46, Qt::KeepAspectRatio, Qt::SmoothTransformation);

            // 创建圆形头像
            QPixmap circularPixmap(46, 46);
            circularPixmap.fill(Qt::transparent);
            QPainter painter(&circularPixmap);
            painter.setRenderHint(QPainter::Antialiasing);
            QPainterPath path;
            path.addEllipse(0, 0, 46, 46);
            painter.setClipPath(path);
            painter.drawPixmap(0, 0, scaledPixmap);
            painter.end();

            avatarButton->setIcon(QIcon(circularPixmap));
            avatarButton->setIconSize(QSize(46, 46));
            avatarButton->setText("");
        }
        return;
    }

    // 否则从配置文件中加载
    QString avatarStoragePath = getAvatarStoragePath();

    // 检查头像文件是否存在
    if (QFile::exists(avatarStoragePath)) {
        QFile file(avatarStoragePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            QString savedAvatarPath = in.readLine();

            // 检查保存的头像文件是否存在
            if (QFile::exists(savedAvatarPath)) {
                avatarPath = savedAvatarPath;

                // 显示头像
                QPixmap pixmap(avatarPath);
                if (!pixmap.isNull()) {
                    pixmap = cropToSquare(pixmap);
                    QPixmap scaledPixmap = pixmap.scaled(46, 46, Qt::KeepAspectRatio, Qt::SmoothTransformation);

                    // 创建圆形头像
                    QPixmap circularPixmap(46, 46);
                    circularPixmap.fill(Qt::transparent);
                    QPainter painter(&circularPixmap);
                    painter.setRenderHint(QPainter::Antialiasing);
                    QPainterPath path;
                    path.addEllipse(0, 0, 46, 46);
                    painter.setClipPath(path);
                    painter.drawPixmap(0, 0, scaledPixmap);
                    painter.end();

                    avatarButton->setIcon(QIcon(circularPixmap));
                    avatarButton->setIconSize(QSize(46, 46));
                    avatarButton->setText("");
                }
            }
            file.close();
        }
    }
}

void ChatWindow::saveUserAvatar(const QString &avatarPath) {
    this->avatarPath = avatarPath;

    // 保存头像路径到配置文件
    QString avatarStoragePath = getAvatarStoragePath();
    QFile file(avatarStoragePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << avatarPath;
        file.close();
    }
}

QString ChatWindow::getAvatarStoragePath() {
    // 获取应用程序配置目录
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(configPath);

    // 创建目录（如果不存在）
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    // 返回头像配置文件路径
    return dir.filePath("avatar_config.txt");
}

QString ChatWindow::extractUsernameFromMessage(const QString &message) {
    if (message.startsWith("[") && message.contains("]: ")) {
        int endBracket = message.indexOf("]");
        return message.mid(1, endBracket - 1);
    }
    return "未知用户";
}

QPixmap ChatWindow::getUserAvatar(const QString &username) {
    // 如果是当前用户，返回当前用户的头像
    if (username == this->username && !avatarPath.isEmpty()) {
        QPixmap pixmap(avatarPath);
        if (!pixmap.isNull()) {
            return cropToSquare(pixmap);
        }
    }

    // 对于其他用户，返回默认头像或缓存的头像
    if (userAvatars.contains(username)) {
        return cropToSquare(userAvatars[username]);
    }

    // 返回空的pixmap表示没有特定头像
    return QPixmap();
}

QPixmap ChatWindow::cropToSquare(const QPixmap &pixmap) {
    if (pixmap.isNull()) return pixmap;

    int size = qMin(pixmap.width(), pixmap.height());
    int x = (pixmap.width() - size) / 2;
    int y = (pixmap.height() - size) / 2;

    return pixmap.copy(x, y, size, size);
}

void ChatWindow::updateOnlineUserAvatar(const QString &username, const QPixmap &avatar) {
    for (int i = 0; i < onlineUsersList->count(); ++i) {
        QListWidgetItem *item = onlineUsersList->item(i);
        QString itemText = item->text();

        // 检查用户名是否匹配
        if (itemText.startsWith(username + "\n")) {
            QPixmap scaledAvatar = cropToSquare(avatar);
            scaledAvatar = scaledAvatar.scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            item->setIcon(QIcon(scaledAvatar));
            break;
        }
    }
}

void ChatWindow::onSendFile() {
    // 打开文件选择对话框
    QString fileName = QFileDialog::getOpenFileName(this, tr("选择要发送的文件"), "", tr("所有文件 (*)"));

    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::warning(this, "错误", "无法打开文件：" + fileName);
            return;
        }

        // 读取文件内容
        QByteArray fileData = file.readAll();
        file.close();

        // 获取文件名和扩展名
        QFileInfo fileInfo(fileName);
        QString displayName = fileInfo.fileName();
        QString fileExtension = fileInfo.suffix().toLower();

        // 判断文件类型
        bool isImage = (fileExtension == "png" || fileExtension == "jpg" || fileExtension == "jpeg" || fileExtension == "gif" || fileExtension == "bmp");
        bool isVideo = (fileExtension == "mp4" || fileExtension == "avi" || fileExtension == "mov" || fileExtension == "mkv" || fileExtension == "wmv");

        // 将文件编码为base64
        QByteArray base64Data = fileData.toBase64();

        // 构造文件传输消息
        QString fileMessage = QString("[%1]: [FILE]%2[FILENAME]%3[FILEEXTENSION]%4[FILETYPE]%5[FILESIZE]%6[FILEDATA]%7[/FILE]")
                             .arg(username)
                             .arg(displayName)
                             .arg(fileExtension)
                             .arg(isImage ? "image" : (isVideo ? "video" : "other"))
                             .arg(fileData.size())
                             .arg(QString::fromUtf8(base64Data))
                             .arg(isImage ? "[THUMBNAIL]" + generateThumbnail(fileData, isImage) : "");

        // 发送文件消息
        networkManager->sendMessageToAllPeers(fileMessage);

        // 在聊天历史中显示发送的文件
        showSentFile(displayName, fileExtension, fileData.size(), isImage, isVideo);
    }
}

void ChatWindow::showSentFile(const QString &fileName, const QString &fileExtension, qint64 fileSize, bool isImage, bool isVideo) {
    QString timestamp = QTime::currentTime().toString("hh:mm:ss");
    QString fileHtml;

    if (isImage) {
        fileHtml = QString("<div style='margin: 12px 0; display: flex; align-items: flex-start;'>"
                          "<div style='width: 32px; height: 32px; "
                          "background: linear-gradient(135deg, #2196F3, #1976D2); "
                          "border-radius: 16px; margin-right: 8px; display: flex; "
                          "align-items: center; justify-content: center; font-size: 16px; color: white; "
                          "box-shadow: 0 2px 4px rgba(0,0,0,0.1);'>🖼️</div>"
                          "<div style='flex: 1;'>"
                          "<div style='display: flex; align-items: center; margin-bottom: 4px;'>"
                          "<span style='color: #2196F3; font-weight: bold; font-size: 14px;'>%1</span>"
                          "<span style='color: #999; font-size: 11px; margin-left: 8px;'>%2</span>"
                          "</div>"
                          "<div style='background: #e3f2fd; padding: 12px; border-radius: 12px; "
                          "border: 1px solid #bbdefb;'>"
                          "<div style='color: #1565C0;'>"
                          "📸 发送了图片：<b>%3</b> (%4 KB)"
                          "</div>"
                          "<div style='width: 100px; height: 100px; "
                          "background: linear-gradient(135deg, #bbdefb, #90caf9); "
                          "border-radius: 8px; margin-top: 8px; display: flex; "
                          "align-items: center; justify-content: center; color: #0d47a1; "
                          "font-size: 24px;'>"
                          "🖼️"
                          "</div>"
                          "</div>"
                          "</div>"
                          "</div>")
                          .arg(username)
                          .arg(timestamp)
                          .arg(fileName)
                          .arg(fileSize / 1024);
    } else if (isVideo) {
        fileHtml = QString("<div style='margin: 12px 0; display: flex; align-items: flex-start;'>"
                          "<div style='width: 32px; height: 32px; "
                          "background: linear-gradient(135deg, #2196F3, #1976D2); "
                          "border-radius: 16px; margin-right: 8px; display: flex; "
                          "align-items: center; justify-content: center; font-size: 16px; color: white; "
                          "box-shadow: 0 2px 4px rgba(0,0,0,0.1);'>🎬</div>"
                          "<div style='flex: 1;'>"
                          "<div style='display: flex; align-items: center; margin-bottom: 4px;'>"
                          "<span style='color: #2196F3; font-weight: bold; font-size: 14px;'>%1</span>"
                          "<span style='color: #999; font-size: 11px; margin-left: 8px;'>%2</span>"
                          "</div>"
                          "<div style='background: #e3f2fd; padding: 12px; border-radius: 12px; "
                          "border: 1px solid #bbdefb;'>"
                          "<div style='color: #1565C0;'>"
                          "🎬 发送了视频：<b>%3</b> (%4 KB)"
                          "</div>"
                          "<div style='width: 100px; height: 100px; "
                          "background: linear-gradient(135deg, #333, #555); "
                          "border-radius: 8px; margin-top: 8px; display: flex; "
                          "align-items: center; justify-content: center; color: white; "
                          "font-size: 24px;'>"
                          "🎬"
                          "</div>"
                          "</div>"
                          "</div>"
                          "</div>")
                          .arg(username)
                          .arg(timestamp)
                          .arg(fileName)
                          .arg(fileSize / 1024);
    } else {
        fileHtml = QString("<div style='margin: 12px 0; display: flex; align-items: flex-start;'>"
                          "<div style='width: 32px; height: 32px; "
                          "background: linear-gradient(135deg, #2196F3, #1976D2); "
                          "border-radius: 16px; margin-right: 8px; display: flex; "
                          "align-items: center; justify-content: center; font-size: 16px; color: white; "
                          "box-shadow: 0 2px 4px rgba(0,0,0,0.1);'>📁</div>"
                          "<div style='flex: 1;'>"
                          "<div style='display: flex; align-items: center; margin-bottom: 4px;'>"
                          "<span style='color: #2196F3; font-weight: bold; font-size: 14px;'>%1</span>"
                          "<span style='color: #999; font-size: 11px; margin-left: 8px;'>%2</span>"
                          "</div>"
                          "<div style='background: #e3f2fd; padding: 12px; border-radius: 12px; "
                          "border: 1px solid #bbdefb;'>"
                          "<div style='color: #1565C0;'>"
                          "📁 发送了文件：<b>%3</b> (%4 KB)"
                          "</div>"
                          "<div style='width: 100px; height: 100px; "
                          "background: linear-gradient(135deg, #e0e0e0, #f0f0f0); "
                          "border-radius: 8px; margin-top: 8px; display: flex; "
                          "align-items: center; justify-content: center; color: #666; "
                          "font-size: 32px;'>"
                          "📁"
                          "</div>"
                          "</div>"
                          "</div>"
                          "</div>")
                          .arg(username)
                          .arg(timestamp)
                          .arg(fileName)
                          .arg(fileSize / 1024);
    }

    chatHistory->append(fileHtml);
    chatHistory->moveCursor(QTextCursor::End);
}

void ChatWindow::onSaveFile() {
    // 这个方法将在用户点击保存文件链接时调用
    // 实际实现会在 onMessageReceived 中处理
}

void ChatWindow::saveReceivedFile(const QString &sender, const QString &filename) {
    QString fileKey = QString("%1_%2").arg(sender).arg(filename);

    if (receivedFiles.contains(fileKey)) {
        // 解码文件数据
        QByteArray fileData = QByteArray::fromBase64(receivedFiles[fileKey].toLatin1());

        // 打开保存文件对话框
        QString saveFileName = QFileDialog::getSaveFileName(this, tr("保存文件"), filename, tr("所有文件 (*)"));

        if (!saveFileName.isEmpty()) {
            QFile saveFile(saveFileName);
            if (saveFile.open(QIODevice::WriteOnly)) {
                saveFile.write(fileData);
                saveFile.close();

                // 显示保存成功消息
                QMessageBox::information(this, "成功",
                                        QString("文件已保存到：\n%1\n\n大小：%2 KB")
                                        .arg(saveFileName)
                                        .arg(fileData.size() / 1024.0, 0, 'f', 1));
            } else {
                QMessageBox::warning(this, "错误", "无法保存文件：" + saveFileName);
            }
        }

        // 从临时存储中移除
        receivedFiles.remove(fileKey);
    }
}

QString ChatWindow::generateThumbnail(const QByteArray &fileData, bool isImage) {
    if (!isImage) {
        return ""; // 非图片文件不生成缩略图
    }

    QPixmap pixmap;
    pixmap.loadFromData(fileData);

    if (pixmap.isNull()) {
        return "";
    }

    // 生成缩略图
    QPixmap thumbnail = pixmap.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // 转换为base64
    QByteArray thumbData;
    QBuffer buffer(&thumbData);
    buffer.open(QIODevice::WriteOnly);
    thumbnail.save(&buffer, "JPEG", 80); // 使用JPEG格式压缩

    return QString::fromLatin1(thumbData.toBase64().data());
}