#include "loginwindow.h"
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QPainter>
#include <QPainterPath>
#include <QGraphicsDropShadowEffect>
#include <QPixmap>
#include <QFontDatabase>
#include <QTimer>

LoginWindow::LoginWindow(QWidget *parent)
    : QWidget(parent), selectedAvatarPath("")
{
    setWindowTitle("P2P 聊天室 - 登录");
    setFixedSize(420, 600);
    setupUI();
    setupConnections();
    applyStyles();
    loadRecentUsers();
}

LoginWindow::~LoginWindow()
{
}

void LoginWindow::setupUI()
{
    // 创建堆叠窗口
    stackedWidget = new QStackedWidget(this);

    // ========== 登录页面 ==========
    loginPage = new QWidget();
    QVBoxLayout *loginLayout = new QVBoxLayout(loginPage);
    loginLayout->setSpacing(20);
    loginLayout->setContentsMargins(40, 40, 40, 40);

    // Logo和标题
    logoLabel = new QLabel();
    logoLabel->setFixedSize(100, 100);
    logoLabel->setStyleSheet("QLabel { background-color: #4CAF50; border-radius: 50px; }");

    titleLabel = new QLabel("P2P 聊天室");
    titleLabel->setAlignment(Qt::AlignCenter);

    subtitleLabel = new QLabel("连接世界，分享生活");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    subtitleLabel->setStyleSheet("color: #666;");

    // 头像预览
    avatarPreview = new QLabel();
    avatarPreview->setFixedSize(80, 80);
    avatarPreview->setStyleSheet("QLabel { border: 2px solid #ddd; border-radius: 40px; background-color: #f5f5f5; }");
    avatarPreview->setAlignment(Qt::AlignCenter);

    // 用户名选择
    QLabel *userLabel = new QLabel("选择用户名:");
    usernameCombo = new QComboBox();
    usernameCombo->setEditable(true);
    usernameCombo->setPlaceholderText("请输入用户名");

    // 记住我选项
    rememberMeCheck = new QCheckBox("记住我");

    // 登录按钮
    loginButton = new QPushButton("开始聊天");
    loginButton->setFixedHeight(45);

    // 注册切换按钮
    registerSwitchButton = new QPushButton("没有账号？立即注册");
    registerSwitchButton->setStyleSheet("QPushButton { border: none; color: #4CAF50; text-decoration: underline; }");

    // 添加到布局
    loginLayout->addWidget(logoLabel, 0, Qt::AlignCenter);
    loginLayout->addWidget(titleLabel);
    loginLayout->addWidget(subtitleLabel);
    loginLayout->addSpacing(20);
    loginLayout->addWidget(avatarPreview, 0, Qt::AlignCenter);
    loginLayout->addSpacing(20);
    loginLayout->addWidget(userLabel);
    loginLayout->addWidget(usernameCombo);
    loginLayout->addWidget(rememberMeCheck);
    loginLayout->addSpacing(20);
    loginLayout->addWidget(loginButton);
    loginLayout->addWidget(registerSwitchButton, 0, Qt::AlignCenter);

    // ========== 注册页面 ==========
    registerPage = new QWidget();
    QVBoxLayout *registerLayout = new QVBoxLayout(registerPage);
    registerLayout->setSpacing(20);
    registerLayout->setContentsMargins(40, 40, 40, 40);

    registerTitle = new QLabel("创建新账号");
    registerTitle->setAlignment(Qt::AlignCenter);

    // 用户名输入
    QLabel *newUserLabel = new QLabel("用户名:");
    newUsernameEdit = new QLineEdit();
    newUsernameEdit->setPlaceholderText("输入用户名（2-20个字符）");

    // 头像选择
    QLabel *avatarLabel = new QLabel("选择头像:");
    QHBoxLayout *avatarLayout = new QHBoxLayout();
    avatarSelectButton = new QPushButton("选择图片");
    QLabel *avatarNote = new QLabel("（可选）");
    avatarNote->setStyleSheet("color: #999;");
    avatarLayout->addWidget(avatarSelectButton);
    avatarLayout->addWidget(avatarNote);
    avatarLayout->addStretch();

    // 头像预览（注册页面）
    QLabel *registerAvatarPreview = new QLabel();
    registerAvatarPreview->setFixedSize(100, 100);
    registerAvatarPreview->setStyleSheet("QLabel { border: 2px dashed #ccc; border-radius: 50px; background-color: #f9f9f9; }");
    registerAvatarPreview->setAlignment(Qt::AlignCenter);
    registerAvatarPreview->setText("👤");

    // 连接头像选择
    connect(avatarSelectButton, &QPushButton::clicked, this, [this, registerAvatarPreview]() {
        QString fileName = QFileDialog::getOpenFileName(this, "选择头像", "",
                                                       "图片文件 (*.png *.jpg *.jpeg *.bmp *.gif)");
        if (!fileName.isEmpty()) {
            selectedAvatarPath = fileName;
            QPixmap pixmap(fileName);
            if (!pixmap.isNull()) {
                pixmap = pixmap.scaled(100, 100, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
                int size = qMin(pixmap.width(), pixmap.height());
                int x = (pixmap.width() - size) / 2;
                int y = (pixmap.height() - size) / 2;
                pixmap = pixmap.copy(x, y, size, size);
                pixmap = pixmap.scaled(96, 96, Qt::KeepAspectRatio, Qt::SmoothTransformation);

                // 创建圆形头像
                QPixmap circularPixmap(96, 96);
                circularPixmap.fill(Qt::transparent);
                QPainter painter(&circularPixmap);
                painter.setRenderHint(QPainter::Antialiasing);
                QPainterPath path;
                path.addEllipse(0, 0, 96, 96);
                painter.setClipPath(path);
                painter.drawPixmap(0, 0, pixmap);
                painter.end();

                registerAvatarPreview->setPixmap(circularPixmap);
                avatarPreview->setPixmap(circularPixmap.scaled(78, 78, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            }
        }
    });

    // 注册按钮
    registerButton = new QPushButton("注册并登录");
    registerButton->setFixedHeight(45);

    // 登录切换按钮
    loginSwitchButton = new QPushButton("已有账号？立即登录");
    loginSwitchButton->setStyleSheet("QPushButton { border: none; color: #4CAF50; text-decoration: underline; }");

    // 添加到注册布局
    registerLayout->addWidget(registerTitle);
    registerLayout->addSpacing(20);
    registerLayout->addWidget(newUserLabel);
    registerLayout->addWidget(newUsernameEdit);
    registerLayout->addSpacing(10);
    registerLayout->addWidget(avatarLabel);
    registerLayout->addLayout(avatarLayout);
    registerLayout->addWidget(registerAvatarPreview, 0, Qt::AlignCenter);
    registerLayout->addSpacing(30);
    registerLayout->addWidget(registerButton);
    registerLayout->addWidget(loginSwitchButton, 0, Qt::AlignCenter);

    // ========== 添加到堆叠窗口 ==========
    stackedWidget->addWidget(loginPage);
    stackedWidget->addWidget(registerPage);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(stackedWidget);

    // 设置默认页面
    stackedWidget->setCurrentIndex(0);
}

void LoginWindow::setupConnections()
{
    connect(loginButton, &QPushButton::clicked, this, &LoginWindow::onLoginClicked);
    connect(registerButton, &QPushButton::clicked, this, &LoginWindow::onRegisterClicked);
    connect(registerSwitchButton, &QPushButton::clicked, this, &LoginWindow::switchToRegister);
    connect(loginSwitchButton, &QPushButton::clicked, this, &LoginWindow::switchToLogin);
    connect(usernameCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        if (index >= 0) {
            QString username = usernameCombo->itemText(index);
            if (recentUsers.contains(username)) {
                QString avatarPath = recentUsers[username];
                if (!avatarPath.isEmpty() && QFile::exists(avatarPath)) {
                    QPixmap pixmap(avatarPath);
                    if (!pixmap.isNull()) {
                        pixmap = pixmap.scaled(78, 78, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
                        int size = qMin(pixmap.width(), pixmap.height());
                        int x = (pixmap.width() - size) / 2;
                        int y = (pixmap.height() - size) / 2;
                        pixmap = pixmap.copy(x, y, size, size);
                        pixmap = pixmap.scaled(76, 76, Qt::KeepAspectRatio, Qt::SmoothTransformation);

                        QPixmap circularPixmap(78, 78);
                        circularPixmap.fill(Qt::transparent);
                        QPainter painter(&circularPixmap);
                        painter.setRenderHint(QPainter::Antialiasing);
                        QPainterPath path;
                        path.addEllipse(0, 0, 78, 78);
                        painter.setClipPath(path);
                        painter.drawPixmap(1, 1, pixmap);
                        painter.end();

                        avatarPreview->setPixmap(circularPixmap);
                        selectedAvatarPath = avatarPath;
                    }
                }
            }
        }
    });
}

void LoginWindow::applyStyles()
{
    // 设置窗口背景
    setStyleSheet("QWidget { background-color: #ffffff; }");

    // 标题样式
    QFont titleFont("Microsoft YaHei", 24, QFont::Bold);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("color: #333;");

    QFont subtitleFont("Microsoft YaHei", 11);
    subtitleLabel->setFont(subtitleFont);

    // 登录页面标题
    registerTitle->setFont(titleFont);
    registerTitle->setStyleSheet("color: #333;");

    // 输入框样式
    QString inputStyle = "QComboBox, QLineEdit { "
                        "padding: 12px; "
                        "border: 1px solid #ddd; "
                        "border-radius: 8px; "
                        "font-size: 14px; "
                        "background-color: #f9f9f9; "
                        "}"
                        "QComboBox:hover, QLineEdit:hover { "
                        "border-color: #4CAF50; "
                        "}"
                        "QComboBox:focus, QLineEdit:focus { "
                        "border-color: #4CAF50; "
                        "border-width: 2px; "
                        "background-color: #fff; "
                        "}"
                        "QComboBox::drop-down { "
                        "border: none; "
                        "}"
                        "QComboBox::down-arrow { "
                        "image: url(:/icons/down_arrow.png); "
                        "width: 12px; "
                        "height: 12px; "
                        "}";

    usernameCombo->setStyleSheet(inputStyle);
    newUsernameEdit->setStyleSheet(inputStyle);

    // 按钮样式
    QString buttonStyle = "QPushButton { "
                         "padding: 12px; "
                         "border: none; "
                         "border-radius: 8px; "
                         "font-size: 16px; "
                         "font-weight: bold; "
                         "color: white; "
                         "background-color: #4CAF50; "
                         "}"
                         "QPushButton:hover { "
                         "background-color: #45a049; "
                         "}"
                         "QPushButton:pressed { "
                         "background-color: #3d8b40; "
                         "}";

    loginButton->setStyleSheet(buttonStyle);
    registerButton->setStyleSheet(buttonStyle);

    // 头像选择按钮样式
    QString avatarButtonStyle = "QPushButton { "
                               "padding: 8px 16px; "
                               "border: 1px solid #4CAF50; "
                               "border-radius: 6px; "
                               "color: #4CAF50; "
                               "background-color: white; "
                               "}"
                               "QPushButton:hover { "
                               "background-color: #f1f8e9; "
                               "}";

    avatarSelectButton->setStyleSheet(avatarButtonStyle);

    // 复选框样式
    rememberMeCheck->setStyleSheet("QCheckBox { color: #666; font-size: 14px; }"
                                  "QCheckBox::indicator { width: 18px; height: 18px; }");

    // 添加阴影效果到Logo
    QGraphicsDropShadowEffect *shadowEffect = new QGraphicsDropShadowEffect();
    shadowEffect->setBlurRadius(20);
    shadowEffect->setColor(QColor(0, 0, 0, 60));
    shadowEffect->setOffset(0, 4);
    logoLabel->setGraphicsEffect(shadowEffect);

    // 创建Logo内容
    QPixmap logoPixmap(100, 100);
    logoPixmap.fill(Qt::transparent);
    QPainter painter(&logoPixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制绿色背景
    painter.setBrush(QColor(76, 175, 80));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(0, 0, 100, 100);

    // 绘制聊天图标
    painter.setBrush(Qt::white);
    painter.drawEllipse(30, 25, 40, 40);

    painter.setBrush(QColor(76, 175, 80));
    painter.drawEllipse(35, 30, 30, 30);

    painter.setBrush(Qt::white);
    painter.drawEllipse(42, 37, 16, 16);

    painter.end();
    logoLabel->setPixmap(logoPixmap);
}

void LoginWindow::loadRecentUsers()
{
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(configPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QString userFile = dir.filePath("users.txt");
    if (QFile::exists(userFile)) {
        QFile file(userFile);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            while (!in.atEnd()) {
                QString line = in.readLine();
                QStringList parts = line.split("|||");
                if (parts.size() >= 2) {
                    QString username = parts[0];
                    QString avatarPath = parts[1];
                    recentUsers[username] = avatarPath;
                    usernameCombo->addItem(username);
                }
            }
            file.close();
        }
    }
}

void LoginWindow::onLoginClicked()
{
    QString username = usernameCombo->currentText().trimmed();

    if (username.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入用户名！");
        return;
    }

    if (username.length() < 2 || username.length() > 20) {
        QMessageBox::warning(this, "提示", "用户名长度应为2-20个字符！");
        return;
    }

    // 保存用户信息（如果选择了记住我）
    if (rememberMeCheck->isChecked()) {
        saveUserInfo(username, selectedAvatarPath);
    }

    emit loginSuccess(username, selectedAvatarPath);
}

void LoginWindow::onRegisterClicked()
{
    QString username = newUsernameEdit->text().trimmed();

    if (username.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入用户名！");
        return;
    }

    if (username.length() < 2 || username.length() > 20) {
        QMessageBox::warning(this, "提示", "用户名长度应为2-20个字符！");
        return;
    }

    // 检查用户名是否已存在
    if (recentUsers.contains(username)) {
        QMessageBox::warning(this, "提示", "该用户名已存在！");
        return;
    }

    // 保存用户信息
    saveUserInfo(username, selectedAvatarPath);

    // 切换到登录页面并选择新用户
    switchToLogin();
    usernameCombo->setCurrentText(username);

    QMessageBox::information(this, "成功", "注册成功！");
    emit loginSuccess(username, selectedAvatarPath);
}

void LoginWindow::onAvatarSelectClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "选择头像", "",
                                                   "图片文件 (*.png *.jpg *.jpeg *.bmp *.gif)");
    if (!fileName.isEmpty()) {
        selectedAvatarPath = fileName;
        // 这里可以添加头像预览更新逻辑
    }
}

void LoginWindow::switchToRegister()
{
    stackedWidget->setCurrentIndex(1);
    newUsernameEdit->clear();
    selectedAvatarPath = "";
}

void LoginWindow::switchToLogin()
{
    stackedWidget->setCurrentIndex(0);
}

void LoginWindow::saveUserInfo(const QString &username, const QString &avatarPath)
{
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(configPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QString userFile = dir.filePath("users.txt");

    // 读取现有用户
    QMap<QString, QString> users;
    if (QFile::exists(userFile)) {
        QFile file(userFile);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            while (!in.atEnd()) {
                QString line = in.readLine();
                QStringList parts = line.split("|||");
                if (parts.size() >= 2) {
                    users[parts[0]] = parts[1];
                }
            }
            file.close();
        }
    }

    // 更新用户信息
    users[username] = avatarPath;

    // 保存到文件
    QFile file(userFile);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        for (auto it = users.begin(); it != users.end(); ++it) {
            out << it.key() << "|||" << it.value() << "\n";
        }
        file.close();
    }

    // 更新最近用户列表
    recentUsers = users;
    usernameCombo->clear();
    for (auto it = users.begin(); it != users.end(); ++it) {
        usernameCombo->addItem(it.key());
    }
}