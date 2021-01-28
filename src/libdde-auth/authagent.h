#ifndef AUTHAGENT_H
#define AUTHAGENT_H

#include <QObject>
#include <atomic>

class DeepinAuthFramework;
class AuthAgent : public QObject {
    Q_OBJECT
public:
    enum AuthFlag {
        keyboard = 1 << 0,
        Fingerprint = 1 << 1,
        Face = 1 << 2,
        ActiveDirectory = 1 << 3
    };

    enum FingerprintStatus {
        MATCH = 0,
        NO_MATCH,
        ERROR,
        RETRY,
        DISCONNECTED
    };

    enum FpRetryStatus {
        SWIPE_TOO_SHORT = 1,
        FINGER_NOT_CENTERED,
        REMOVE_AND_RETRY
    };

    explicit AuthAgent(DeepinAuthFramework *deepin);
    ~AuthAgent();

    void Responsed(const QString& password);
    QString Password() { return m_password; }
    void Authenticate(const QString& username);
    int GetAuthType();
    DeepinAuthFramework *deepinAuth() { return m_deepinauth; }
    void setCancelAuth(bool isCancel) { m_isCancel = isCancel; }

signals:
    void requestEchoOff(const QString &msg);
    void requestEchoOn(const QString &msg);
    void displayErrorMsg(const QString &msg);
    void displayTextInfo(const QString &msg);
    void respondResult(const QString &msg);

private:
    static int pamConversation(int num,
                               const struct pam_message** msg,
                               struct pam_response** resp,
                               void* app_data);

private:
    DeepinAuthFramework* m_deepinauth = nullptr;
    //添加volatile类型修饰符，告知编译器该变量可以被某些未知的因素更改
    //所以对访问该变量的代码就不再进行优化从而可以提供对特殊地址的稳定访问
    bool volatile m_isCondition = true;
    //增加变量判断是否取消验证,以退出等待输入密码循环
    bool volatile m_isCancel = false;

    QString m_password;
    AuthFlag m_authType;
    QString m_userName;
};

#endif // AUTHAGENT_H
