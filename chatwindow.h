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
#include <QToolButton>
#include <QMenu>
#include <QGridLayout>
#include <QScrollArea>
#include <QFont>
#include <QMap>
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

    void handleFileMessage(const QString &message);

    void onPeerDiscovered(const QString &ip, const QString &username);
    void insertEmoji(const QString &emoji);
    void onAvatarButtonClicked();
    void onSendFile();

    void showSentFile(const QString &fileName, const QString &fileExtension, qint64 fileSize, bool isImage,
                      bool isVideo);

    void onSaveFile();

    void saveReceivedFile(const QString &sender, const QString &filename);

private:
    void setupUI();
    void setupConnections();
    void createEmojiMenu();
    void initEmojiMap();
    QString processMessageWithEmojis(const QString &message);

    QVBoxLayout *mainLayout;
    QTextEdit *chatHistory;
    QLineEdit *messageInput;
    QPushButton *sendButton;
    QToolButton *emojiButton;
    QListWidget *onlineUsersList;
    QLabel *statusLabel;

    QMenu *emojiMenu;

    NetworkManager *networkManager;
    QString username;

    // 表情映射：文本代码 -> 表情符号
    QMap<QString, QString> emojiMap;

    // 常用表情列表
    QStringList commonEmojis;

    QString avatarPath;           // 存储头像路径
    QPushButton *avatarButton;    // 头像设置按钮

    // 添加新的私有方法
    void loadUserAvatar();
    void saveUserAvatar(const QString &avatarPath);
    QString getAvatarStoragePath();

    // 添加用户头像缓存
    QMap<QString, QPixmap> userAvatars;  // 用户名 -> 头像映射
    // 添加新的方法
    QString extractUsernameFromMessage(const QString &message);
    QPixmap getUserAvatar(const QString &username);

    QPixmap cropToSquare(const QPixmap &pixmap);
    void updateOnlineUserAvatar(const QString &username, const QPixmap &avatar);
    QPushButton *fileButton;  // 文件发送按钮
    QString currentFilePath;  // 当前正在传输的文件路径
    QMap<QString, QString> receivedFiles;  // 接收的文件信息：用户名 -> 文件路径
};

#endif // CHATWINDOW_H