#include <QApplication>
#include "loginwindow.h"
#include "chatwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // 设置应用程序信息
    app.setApplicationName("P2P Chat");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("ChatSoft");

    // 创建登录窗口
    LoginWindow loginWindow;
    ChatWindow *chatWindow = nullptr;

    // 连接登录成功信号
    QObject::connect(&loginWindow, &LoginWindow::loginSuccess,
                    [&](const QString &username, const QString &avatarPath) {
        // 创建聊天窗口，传入用户名和头像路径
        chatWindow = new ChatWindow(username, avatarPath);

        // 显示聊天窗口
        chatWindow->show();

        // 隐藏登录窗口
        loginWindow.hide();
    });

    // 显示登录窗口
    loginWindow.show();

    int result = app.exec();

    // 清理资源
    if (chatWindow) {
        delete chatWindow;
    }

    return result;
}