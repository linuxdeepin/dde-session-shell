#ifndef DEEPINAUTHFRAMEWORK_H
#define DEEPINAUTHFRAMEWORK_H

#include <QObject>
#include <QPointer>

#include <com_deepin_daemon_authenticate.h>
#include <com_deepin_daemon_authenticate_session2.h>
#include <memory>

using AuthInter = com::deepin::daemon::Authenticate;
using AuthControllerInter = com::deepin::daemon::authenticate::Session;

typedef void *(* FUNC_BIO_S_MEM)(); // 定义函数指针类型的别名
typedef void *(* FUNC_BIO_NEW)(void *); // 定义函数指针类型的别名
typedef int (* FUNC_BIO_PUTS)(void *,const char *); // 定义函数指针类型，返回类型是整形的别名
typedef void* (* PEM_READ_BIO_RSA_PUBKEY)(void *, void *,void *,void *); // 定义函数指针类型。有四个void*参数的别名
typedef void* (* PEM_READ_BIO_RSAPUBLICKEY)(void *, void *,void *,void *); // 定义函数指针类型的有四个void*参数的别名
typedef void* (* RSA_PUBLIC_ENCRYPT)(int flen, const unsigned char *from,unsigned char *to, void *rsa, int padding); // 定义函数指针类型且有多个参数的的别名
//typedef int (* FUNC_RSA_SIZE)(void *);
typedef void (* FUNC_RSA_FREE)(void *);// 定义函数指针类型的别名
typedef void *d_RSA; // 定义结构体类型的别名

class DeepinAuthInterface;
class QThread;
class User;
class DeepinAuthFramework : public QObject
{
    Q_OBJECT
public:
    enum AuthQuitFlag {
        AutoQuit,  // 自动退出（默认）
        ManualQuit // 手动退出
    };
    Q_ENUM(AuthQuitFlag)

    enum AuthFlag {
        Password = 1 << 0,
        Fingerprint = 1 << 1,
        Face = 1 << 2,
        ActiveDirectory = 1 << 3
    };
    Q_ENUM(AuthFlag)

    explicit DeepinAuthFramework(DeepinAuthInterface *interface, QObject *parent = nullptr);
    ~DeepinAuthFramework();

    /* com.deepin.daemon.Authenticate */
    int GetFrameworkState() const;
    int GetSupportedMixAuthFlags() const;
    QString GetPreOneKeyLogin(const int flag) const;
    QString GetLimitedInfo(const QString &account) const;
    QString GetSupportedEncrypts() const;
    /* com.deepin.daemon.Authenticate.Session */
    int GetFuzzyMFA(const QString &account) const;
    int GetMFAFlag(const QString &account) const;
    int GetPINLen(const QString &account) const;
    MFAInfoList GetFactorsInfo(const QString &account) const;
    QString GetPrompt(const QString &account) const;

    QString AuthSessionPath(const QString &account) const;

    void cancelAuthenticate();

signals:
    /* com.deepin.daemon.Authenticate */
    void LimitedInfoChanged(const QString &);
    void SupportedMixAuthFlagsChanged(const int);
    void FramworkStateChanged(const int);
    void SupportedEncryptsChanged(const QString &);
    /* com.deepin.daemon.Authenticate.Session */
    void MFAFlagChanged(const bool);
    void FuzzyMFAChanged(const bool);
    void PromptChanged(const QString &);
    void FactorsInfoChanged(const MFAInfoList &);
    void PINLenChanged(const int);
    void AuthStatusChanged(int flag, int status, const QString &result);

public slots:
    /* Compatible with old authentication methods */
    void Authenticate(const QString &account);
    void Responsed(const QString &password);
    void DisplayErrorMsg(const QString &msg);
    void DisplayTextInfo(const QString &msg);
    void RespondResult(const QString &msg);
    const QString &RequestEchoOff(const QString &msg);
    /* New authentication framework */
    void CreateAuthController(const QString &account, const int authType, const int encryptType);
    void DestoryAuthController(const QString &account);
    void StartAuthentication(const QString &account, const int authType, const int timeout);
    void EndAuthentication(const QString &account, const int authType);
    void SendTokenToAuth(const QString &account, const int authType, const QString &token);
    void SetAuthQuitFlag(const QString &account, const int flag = AutoQuit);

private:
    /* Compatible with old authentication methods */
    static void *PAMAuthWorker(void *arg);
    void PAMAuthentication(const QString &account);
    static int PAMConversation(int num_msg, const struct pam_message **msg, struct pam_response **resp, void *app_data);

    void initPublicKey(const QString &account);

private:
    DeepinAuthInterface *m_interface;
    AuthInter *m_authenticateInter;
    pthread_t m_PAMAuthThread;
    QString m_account;
    AuthFlag m_authType;
    QString m_password;
    QMap<QString, AuthControllerInter *> *m_authenticateControllers;
    const char * m_pubkey = nullptr;
};

#endif // DEEPINAUTHFRAMEWORK_H
