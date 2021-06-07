#ifndef DEEPINAUTHFRAMEWORK_H
#define DEEPINAUTHFRAMEWORK_H

#include <QObject>
#include <QPointer>

#include <com_deepin_daemon_authenticate.h>
#include <com_deepin_daemon_authenticate_session2.h>
#include <memory>

using AuthInter = com::deepin::daemon::Authenticate;
using AuthControllerInter = com::deepin::daemon::authenticate::Session;

using FUNC_BIO_S_MEM = void *(*)();
using FUNC_BIO_NEW = void *(*)(void *);
using FUNC_BIO_PUTS = int (*)(void *, const char *);
using FUNC_PEM_READ_BIO_RSA_PUBKEY = void *(*)(void *, void *, void *, void *);
using FUNC_PEM_READ_BIO_RSAPUBLICKEY = void *(*)(void *, void *, void *, void *);
using FUNC_RSA_PUBLIC_ENCRYPT = void *(*)(int flen, const unsigned char *from, unsigned char *to, void *rsa, int padding);
using FUNC_RSA_SIZE = int (*)(void *);
using FUNC_RSA_FREE = void (*)(void *);

class DeepinAuthInterface;
class QThread;
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

    /* Compatible with old authentication methods */
    void CreateAuthenticate(const QString &account);
    void SendToken(const QString &token);
    void DestoryAuthenticate();

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
    void setEncryption(const int type, const ArrayInt method);

signals:
    /* com.deepin.daemon.Authenticate */
    void LimitsInfoChanged(const QString &);
    void SupportedMixAuthFlagsChanged(const int);
    void FramworkStateChanged(const int);
    void SupportedEncryptsChanged(const QString &);
    /* com.deepin.daemon.Authenticate.Session */
    void MFAFlagChanged(const bool);
    void FuzzyMFAChanged(const bool);
    void PromptChanged(const QString &);
    void FactorsInfoChanged(const MFAInfoList &);
    void PINLenChanged(const int);
    void AuthStatusChanged(const int, const int, const QString &);

public slots:
    /* New authentication framework */
    void CreateAuthController(const QString &account, const int authType, const int appType);
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
    void UpdateAuthStatus(const int status, const QString &message);

    void initEncryptionService();

private:
    DeepinAuthInterface *m_interface;
    AuthInter *m_authenticateInter;
    pthread_t m_PAMAuthThread;
    QString m_account;
    QString m_message;
    QString m_token;
    int m_encryptType;
    QString m_publicKey;
    ArrayInt m_encryptMethod;
    AuthFlag m_authType;
    QMap<QString, AuthControllerInter *> *m_authenticateControllers;
    bool m_cancelAuth;
    bool m_waitToken;

    void *m_encryptionHandle;
    FUNC_BIO_NEW m_F_BIO_new;
    FUNC_BIO_PUTS m_F_BIO_puts;
    FUNC_BIO_S_MEM m_F_BIO_s_mem;
    FUNC_PEM_READ_BIO_RSAPUBLICKEY m_F_PEM_read_bio_RSAPublicKey;
    FUNC_PEM_READ_BIO_RSA_PUBKEY m_F_PEM_read_bio_RSA_PUBKEY;
    FUNC_RSA_FREE m_F_RSA_free;
    FUNC_RSA_FREE m_F_BIO_free;
    FUNC_RSA_PUBLIC_ENCRYPT m_F_RSA_public_encrypt;
    FUNC_RSA_SIZE m_F_RSA_size;
    void *m_BIO;
    void *m_RSA;
};

#endif // DEEPINAUTHFRAMEWORK_H
