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
    // 修改构造函数，接收用户名和头像路径
    explicit ChatWindow(const QString &username = "", const QString &avatarPath = "", QWidget *parent = nullptr);
    ~ChatWindow();

private slots:
    void onSendMessage();
    void onMessageReceived(const QString &message);
    void handleFileMessage(const QString &message);
    void onPeerDiscovered(const QString &ip, const QString &username);
    void insertEmoji(const QString &emoji);
    void onAvatarButtonClicked();
    void onSendFile();
    void showSentFile(const QString &fileName, const QString &fileExtension, qint64 fileSize, bool isImage, bool isVideo);
    void onSaveFile();
    void saveReceivedFile(const QString &sender, const QString &filename);
    QString generateThumbnail(const QByteArray &fileData, bool isImage);

private:
    void setupUI();
    void setupConnections();
    void createEmojiMenu();
    void initEmojiMap();
    QString processMessageWithEmojis(const QString &message);
    void loadUserAvatar();
    void saveUserAvatar(const QString &avatarPath);
    QString getAvatarStoragePath();
    QString extractUsernameFromMessage(const QString &message);
    QPixmap getUserAvatar(const QString &username);
    QPixmap cropToSquare(const QPixmap &pixmap);
    void updateOnlineUserAvatar(const QString &username, const QPixmap &avatar);

    // 界面控件
    QVBoxLayout *mainLayout{};
    QTextEdit *chatHistory{};
    QLineEdit *messageInput{};
    QPushButton *sendButton{};
    QToolButton *emojiButton{};
    QListWidget *onlineUsersList{};
    QLabel *statusLabel{};
    QMenu *emojiMenu{};
    QPushButton *avatarButton{};
    QPushButton *fileButton{};

    // 网络和用户数据
    NetworkManager *networkManager;
    QString username;
    QString avatarPath;

    // 表情相关
    QMap<QString, QString> emojiMap;
    QStringList commonEmojis;

    // 用户头像缓存
    QMap<QString, QPixmap> userAvatars;

    // 文件传输
    QString currentFilePath;
    QMap<QString, QString> receivedFiles;  // 用户名_文件名 -> base64数据
};

#endif // CHATWINDOW_H