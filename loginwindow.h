#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QCheckBox>
#include <QStackedWidget>
#include <QGroupBox>
#include <QFontDatabase>


class LoginWindow : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();

    signals:
        void loginSuccess(const QString &username, const QString &avatarPath);

private slots:
    void onLoginClicked();
    void onRegisterClicked();
    void onAvatarSelectClicked();
    void switchToRegister();
    void switchToLogin();
    void loadRecentUsers();

private:
    void setupUI();
    void setupConnections();
    void applyStyles();
    void saveUserInfo(const QString &username, const QString &avatarPath);
    void loadUserInfo();

    // 登录界面控件
    QStackedWidget *stackedWidget;

    // 登录页面
    QWidget *loginPage;
    QLabel *logoLabel;
    QLabel *titleLabel;
    QLabel *subtitleLabel;
    QComboBox *usernameCombo;
    QPushButton *loginButton;
    QPushButton *registerSwitchButton;
    QCheckBox *rememberMeCheck;
    QLabel *avatarPreview;

    // 注册页面
    QWidget *registerPage;
    QLabel *registerTitle;
    QLineEdit *newUsernameEdit;
    QPushButton *avatarSelectButton;
    QPushButton *registerButton;
    QPushButton *loginSwitchButton;

    QString selectedAvatarPath;
    QMap<QString, QString> recentUsers; // 用户名 -> 头像路径
};

#endif // LOGINWINDOW_H