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

ChatWindow::ChatWindow(QWidget *parent) : QWidget(parent) {
    // 初始化表情映射
    initEmojiMap();

    setupUI();
    setupConnections();
    createEmojiMenu();

    // 加载用户头像
    loadUserAvatar();

    bool ok;
    username = QInputDialog::getText(this, "用户名", "请输入您的用户名:", QLineEdit::Normal, "", &ok);

    if (!ok || username.isEmpty()) {
        username = "匿名用户";
    }

    networkManager = new NetworkManager(this, username);
    connect(networkManager, &NetworkManager::messageReceived, this, &ChatWindow::onMessageReceived);
    connect(networkManager, &NetworkManager::peerDiscovered, this, &ChatWindow::onPeerDiscovered);
    // 连接头像按钮点击信号
    connect(avatarButton, &QPushButton::clicked, this, &ChatWindow::onAvatarButtonClicked);

    statusLabel->setText(QString("就绪 - 用户名: %1 - 点击😊按钮发送表情").arg(username));
}

ChatWindow::~ChatWindow() {
    delete networkManager;
}

void ChatWindow::initEmojiMap() {
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

    // 头像按钮
    avatarButton = new QPushButton(this);
    avatarButton->setFixedSize(40, 40);
    avatarButton->setStyleSheet("QPushButton { "
                               "border: 2px solid #ccc; "
                               "border-radius: 20px; "
                               "background-color: #f0f0f0; "
                               "font-size: 18px; "
                               "}"
                               "QPushButton:hover { "
                               "background-color: #e0e0e0; "
                               "}");
    avatarButton->setText("👤");
    avatarButton->setToolTip("设置头像");

    // 表情按钮
    emojiButton = new QToolButton(this);
    emojiButton->setText("😊");
    emojiButton->setToolTip("选择表情包");
    emojiButton->setFixedSize(40, 40);
    emojiButton->setStyleSheet("QToolButton { font-size: 20px; border: 1px solid #ccc; border-radius: 5px; background-color: #f0f0f0; }"
                               "QToolButton:hover { background-color: #e0e0e0; }");


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

    inputLayout->addWidget(avatarButton);
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

        // ... 按钮样式设置 ...

        QString emoji = commonEmojis[i];
        connect(emojiBtn, &QPushButton::clicked, this, [this, emoji]() {
            insertEmoji(emoji);
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

    connect(emojiButton, &QToolButton::clicked, this, [this]() {
       emojiMenu->exec(emojiButton->mapToGlobal(QPoint(0, emojiButton->height())));
   });
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
    QString timestamp = QTime::currentTime().toString("hh:mm");

    // 获取当前用户头像
    QString avatarHtml = "";
    if (!avatarData.isEmpty()) {
        avatarHtml = QString("<img src='data:image/png;base64,%1' width='32' height='32' style='vertical-align: middle; margin-right: 5px; border-radius: 16px;' />").arg(avatarData);
    } else {
        // 默认头像
        avatarHtml = "<div style='width: 32px; height: 32px; background-color: #ddd; border-radius: 16px; margin-right: 5px; display: flex; align-items: center; justify-content: center; font-size: 16px;'>👤</div>";
    }

    QString displayMessage = QString("<div style='margin: 5px 0; display: flex; align-items: flex-start;'>"
                                 "%1"
                                 "<div>"
                                 "<div><span style='color: #666; font-size: 11px;'>[%2]</span> "
                                 "<span style='color: #2196F3; font-weight: bold;'>%3:</span></div>"
                                 "<div style='font-size: 14px; margin-top: 2px;'>%4</div>"
                                 "</div>"
                                 "</div>")
                                 .arg(avatarHtml)
                                 .arg(timestamp)
                                 .arg(username)
                                 .arg(processedMessage.toHtmlEscaped().replace("\n", "<br>"));

    chatHistory->append(displayMessage);
    chatHistory->moveCursor(QTextCursor::End);

    messageInput->clear();

    // 发送带头像信息的消息
    networkManager->sendMessageToAllPeers(fullMessage);

}

void ChatWindow::onMessageReceived(const QString &message) {
     // 提取用户名和消息内容
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
    QString timestamp = QTime::currentTime().toString("hh:mm");

    // 获取发送者头像
    QString avatarHtml = "";
    if (!avatarData.isEmpty()) {
        avatarHtml = QString("<img src='data:image/png;base64,%1' width='32' height='32' style='vertical-align: middle; margin-right: 5px; border-radius: 16px;' />").arg(avatarData);
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
            avatarHtml = QString("<img src='data:image/png;base64,%1' width='32' height='32' style='vertical-align: middle; margin-right: 5px; border-radius: 16px;' />").arg(base64Image);
        } else {
            // 默认头像
            avatarHtml = "<div style='width: 32px; height: 32px; background-color: #ddd; border-radius: 16px; margin-right: 5px; display: flex; align-items: center; justify-content: center; font-size: 16px;'>👤</div>";
        }
    }

    QString fullMessage = QString("<div style='margin: 5px 0; display: flex; align-items: flex-start;'>"
                                 "%1"
                                 "<div>"
                                 "<div><span style='color: #666; font-size: 11px;'>[%2]</span> "
                                 "<span style='color: #4CAF50; font-weight: bold;'>%3:</span></div>"
                                 "<div style='font-size: 14px; margin-top: 2px;'>%4</div>"
                                 "</div>"
                                 "</div>")
                                 .arg(avatarHtml)
                                 .arg(timestamp)
                                 .arg(senderUsername)
                                 .arg(displayMessage.toHtmlEscaped().replace("\n", "<br>"));

    chatHistory->append(fullMessage);
    chatHistory->moveCursor(QTextCursor::End);
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
        existingItem->setForeground(QColor("#2e7d32")); // 绿色表示在线

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
    item->setForeground(QColor("#2e7d32")); // 绿色
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
        painter.setBrush(QColor("#ddd"));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(0, 0, 24, 24);
        painter.setPen(QColor("#666"));
        painter.setFont(QFont("Segoe UI", 12));
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
    // // 如果已经设置了头像，则不允許再次設置
    // if (!avatarPath.isEmpty()) {
    //     QMessageBox::information(this, "提示", "您已经设置了头像，无法再次修改！");
    //     return;
    // }

    // 打开文件选择对话框
    QString fileName = QFileDialog::getOpenFileName(this,
                                                   tr("选择头像"),
                                                   "",
                                                   tr("Image Files (*.png *.jpg *.bmp *.jpeg *.gif)"));

    if (!fileName.isEmpty()) {
        // 加载并裁剪头像
        QPixmap pixmap(fileName);
        if (!pixmap.isNull()) {
            pixmap = cropToSquare(pixmap);
            // 保存头像
            saveUserAvatar(fileName);

            // 显示头像
            QPixmap scaledPixmap = pixmap.scaled(40, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            avatarButton->setIcon(QIcon(scaledPixmap));
            avatarButton->setIconSize(QSize(40, 40));
            avatarButton->setText(""); // 清除文字
        }
    }
}

void ChatWindow::loadUserAvatar() {
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
                    QPixmap scaledPixmap = pixmap.scaled(40, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                    avatarButton->setIcon(QIcon(scaledPixmap));
                    avatarButton->setIconSize(QSize(40, 40));
                    avatarButton->setText(""); // 清除文字
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