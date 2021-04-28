#include "deepinauthframework.h"
#include "interface/deepinauthinterface.h"
#include "userinfo.h"

#include <QThread>
#include <QTimer>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <signal.h>
#include <security/pam_appl.h>

#ifdef PAM_SUN_CODEBASE
#define PAM_MSG_MEMBER(msg, n, member) ((*(msg))[(n)].member)
#else
#define PAM_MSG_MEMBER(msg, n, member) ((msg)[(n)]->member)
#endif

#define PAM_SERVICE_SYSTEM_NAME "password-auth"
#define PAM_SERVICE_DEEPIN_NAME "common-auth"

//dlopen
#include <dlfcn.h>

#define PKCS1_HEADER "-----BEGIN RSA PUBLIC KEY-----"
#define PKCS8_HEADER "-----BEGIN PUBLIC KEY-----"
#define AUTHRNTICATESERVICE "com.deepin.daemon.Authenticate"
#define AUTHRNTICATEINTERFACE "com.deepin.daemon.Authenticate.Session"
#define OPENSSLNAME "libssl.so"

DeepinAuthFramework::DeepinAuthFramework(DeepinAuthInterface *interface, QObject *parent)
    : QObject(parent)
    , m_interface(interface)
    , m_authenticateInter(new AuthInter(AUTHRNTICATESERVICE, "/com/deepin/daemon/Authenticate", QDBusConnection::systemBus(), this))
    , m_PAMAuthThread(0)
    , m_authenticateControllers(new QMap<QString, AuthControllerInter *>())
{
    connect(m_authenticateInter, &AuthInter::FrameworkStateChanged, this, &DeepinAuthFramework::FramworkStateChanged);
    connect(m_authenticateInter, &AuthInter::LimitUpdated, this, [=](const QString &value) {
        LimitedInfoChanged(m_authenticateInter->GetLimits(value));
    });
    connect(m_authenticateInter, &AuthInter::SupportedFlagsChanged, this, &DeepinAuthFramework::SupportedMixAuthFlagsChanged);
    connect(m_authenticateInter, &AuthInter::SupportEncryptsChanged, this, &DeepinAuthFramework::SupportedEncryptsChanged);
}

DeepinAuthFramework::~DeepinAuthFramework()
{
    for (const QString &key : m_authenticateControllers->keys()) {
        m_authenticateControllers->remove(key);
    }
    delete m_authenticateControllers;
}

/**
 * @brief 开启认证服务
 *
 * @param user
 */
void DeepinAuthFramework::Authenticate(const QString &account)
{
    m_account = account;
    if (m_PAMAuthThread != 0) {
        pthread_cancel(m_PAMAuthThread);
        pthread_join(m_PAMAuthThread, nullptr);
        m_PAMAuthThread = 0;
    }
    int rc = pthread_create(&m_PAMAuthThread, nullptr, &PAMAuthWorker, this);
    if (rc != 0) {
        qCritical() << "failed to create the authentication thread: %s" << strerror(errno);
    }
}

void *DeepinAuthFramework::PAMAuthWorker(void *arg)
{
    DeepinAuthFramework *authFramework = static_cast<DeepinAuthFramework *>(arg);
    if (authFramework != nullptr) {
        authFramework->PAMAuthentication(authFramework->m_account);
    } else {
        qCritical() << "pam auth worker deepin framework is nullptr";
    }
    return nullptr;
}

void DeepinAuthFramework::PAMAuthentication(const QString &account)
{
    pam_handle_t *m_pamHandle = nullptr;
    pam_conv conv = {PAMConversation, static_cast<void *>(this)};
    const char *serviceName = isDeepinAuth() ? PAM_SERVICE_DEEPIN_NAME : PAM_SERVICE_SYSTEM_NAME;
    int ret = pam_start(serviceName, account.toLocal8Bit().data(), &conv, &m_pamHandle);

    if (ret != PAM_SUCCESS) {
        qCritical() << "pam_start() failed: " << pam_strerror(m_pamHandle, ret);
    }

    int rc = pam_authenticate(m_pamHandle, 0);
    if (rc != PAM_SUCCESS) {
        qCritical() << "pam_authenticate() failed: " << pam_strerror(m_pamHandle, rc) << "####" << rc;
    }

    int re = pam_end(m_pamHandle, rc);
    if (re != PAM_SUCCESS) {
        qCritical() << "pam_end() failed: " << pam_strerror(m_pamHandle, re);
    }

    RespondResult((rc == PAM_SUCCESS && re == PAM_SUCCESS) ? "success" : QString());

    system("xset dpms force on");
}

/**
 * @brief PAM 的回调函数
 *
 * @param num_msg
 * @param msg
 * @param resp
 * @param app_data
 * @return int
 */
int DeepinAuthFramework::PAMConversation(int num_msg, const pam_message **msg, pam_response **resp, void *app_data)
{
    DeepinAuthFramework *app_ptr = static_cast<DeepinAuthFramework *>(app_data);
    struct pam_response *aresp = nullptr;
    int idx = 0;

    QPointer<DeepinAuthFramework> isThreadAlive(app_ptr);
    if (!isThreadAlive) {
        qDebug() << "pam: application is null";
        return PAM_CONV_ERR;
    }

    if (num_msg <= 0 || num_msg > PAM_MAX_NUM_MSG)
        return PAM_CONV_ERR;

    if ((aresp = static_cast<struct pam_response *>(calloc(num_msg, sizeof(*aresp)))) == nullptr)
        return PAM_BUF_ERR;

    for (idx = 0; idx < num_msg; ++idx) {
        switch (PAM_MSG_MEMBER(msg, idx, msg_style)) {
        case PAM_PROMPT_ECHO_OFF: {
            app_ptr->DisplayTextInfo(QString::fromLocal8Bit(PAM_MSG_MEMBER(msg, idx, msg)));

            // while (app_ptr->m_isCondition) {
            //     //取消验证时返回一般错误,退出等待输入密码的循环,然后退出验证线程
            //     if (app_ptr->m_isCancel) {
            //         return PAM_ABORT;
            //     }
            //     sleep(1);
            // }

            // app_ptr->m_isCondition = true;

            if (!QPointer<DeepinAuthFramework>(app_ptr)) {
                qDebug() << "pam: deepin auth framework is null";
                return PAM_CONV_ERR;
            }

            QString password = app_ptr->RequestEchoOff(PAM_MSG_MEMBER(msg, idx, msg));
            aresp[idx].resp = strdup(password.toLocal8Bit().data());

            if (aresp[idx].resp == nullptr)
                goto fail;

            app_ptr->m_authType = AuthFlag::Password;
            aresp[idx].resp_retcode = PAM_SUCCESS;
            break;
        }

        case PAM_PROMPT_ECHO_ON:
        case PAM_ERROR_MSG: {
            qDebug() << "pam authagent error: " << PAM_MSG_MEMBER(msg, idx, msg);
            app_ptr->DisplayErrorMsg(QString::fromLocal8Bit(PAM_MSG_MEMBER(msg, idx, msg)));
            app_ptr->m_authType = AuthFlag::Fingerprint;
            aresp[idx].resp_retcode = PAM_SUCCESS;
            break;
        }

        case PAM_TEXT_INFO: {
            qDebug() << "pam authagent info: " << PAM_MSG_MEMBER(msg, idx, msg);
            app_ptr->DisplayTextInfo(QString::fromLocal8Bit(PAM_MSG_MEMBER(msg, idx, msg)));
            aresp[idx].resp_retcode = PAM_SUCCESS;
            break;
        }

        default:
            goto fail;
        }
    }

    *resp = aresp;

    return PAM_SUCCESS;

fail:
    for (idx = 0; idx < num_msg; idx++) {
        free(aresp[idx].resp);
    }
    free(aresp);
    return PAM_CONV_ERR;
}

void DeepinAuthFramework::Responsed(const QString &password)
{
    if (password == m_password) {
        return;
    }
    m_password = password;
}

/**
 * @brief 显示错误信息
 *
 * @param msg
 */
void DeepinAuthFramework::DisplayErrorMsg(const QString &msg)
{
    m_interface->onDisplayErrorMsg(msg);
}

/**
 * @brief 显示提示文案
 *
 * @param msg
 */
void DeepinAuthFramework::DisplayTextInfo(const QString &msg)
{
    m_interface->onDisplayTextInfo(msg);
}

void DeepinAuthFramework::initPublicKey(const QString &account)
{
    QVector<int> encryptMethod = {1};
    QDBusInterface ifc(AUTHRNTICATESERVICE, AuthSessionPath(account), AUTHRNTICATEINTERFACE, QDBusConnection::systemBus(), nullptr);
    QDBusPendingReply<int, QVector<int>, QString> reply = ifc.call("EncryptKey", 0, QVariant::fromValue(encryptMethod));
    reply.waitForFinished();

    if (reply.isError()) {
        qInfo() << AuthSessionPath(account) << "Authenticate EncryptKey interface error msg:" << reply.error().message();
    }

    m_pubkey = reply.argumentAt(2).toString().toUtf8();
}

/**
 * @brief 显示认证结果
 *
 * @param msg
 */
void DeepinAuthFramework::RespondResult(const QString &msg)
{
    m_interface->onPasswordResult(msg);
}

const QString &DeepinAuthFramework::RequestEchoOff(const QString &msg)
{
    Q_UNUSED(msg);

    return m_password;
}

/**
 * @brief 创建认证服务
 *
 * @param account     用户名
 * @param authType    认证方式（多因、单因，一种或多种）
 * @param encryptType 加密方式
 */
void DeepinAuthFramework::CreateAuthController(const QString &account, const int authType, const int appType)
{
    if (m_authenticateControllers->contains(account) && m_authenticateControllers->value(account)->isValid()) {
        return;
    }
    qInfo() << "Create Authenticate Session:" << account << authType << appType;
    const QString authControllerInterPath = m_authenticateInter->Authenticate(account, authType, appType);
    AuthControllerInter *authControllerInter = new AuthControllerInter("com.deepin.daemon.Authenticate", authControllerInterPath, QDBusConnection::systemBus(), this);
    m_authenticateControllers->insert(account, authControllerInter);
    // authControllerInter->EncryptKey(); // TODO 获取公钥
    connect(authControllerInter, &AuthControllerInter::FactorsInfoChanged, this, &DeepinAuthFramework::FactorsInfoChanged);
    connect(authControllerInter, &AuthControllerInter::IsFuzzyMFAChanged, this, &DeepinAuthFramework::FuzzyMFAChanged);
    connect(authControllerInter, &AuthControllerInter::IsMFAChanged, this, &DeepinAuthFramework::MFAFlagChanged);
    connect(authControllerInter, &AuthControllerInter::PINLenChanged, this, &DeepinAuthFramework::PINLenChanged);
    connect(authControllerInter, &AuthControllerInter::PromptChanged, this, &DeepinAuthFramework::PromptChanged);
    connect(authControllerInter, &AuthControllerInter::Status, this, &DeepinAuthFramework::AuthStatusChanged);

    emit FactorsInfoChanged(authControllerInter->factorsInfo());
    emit FuzzyMFAChanged(authControllerInter->isFuzzyMFA());
    emit MFAFlagChanged(authControllerInter->isMFA());
    emit PINLenChanged(authControllerInter->pINLen());
    emit PromptChanged(authControllerInter->prompt());

    // initPublicKey(account);
}

/**
 * @brief 销毁认证服务，下次使用认证服务前需要先创建
 *
 * @param account 用户名
 */
void DeepinAuthFramework::DestoryAuthController(const QString &account)
{
    if (!m_authenticateControllers->contains(account)) {
        return;
    }
    qInfo() << "Destory Authenticate Sesssion:" << account;
    AuthControllerInter *authControllerInter = m_authenticateControllers->value(account);
    authControllerInter->End(-1);
    authControllerInter->Quit();
    m_authenticateControllers->remove(account);
}

/**
 * @brief 开启认证服务。成功开启返回0,否则返回失败个数。
 *
 * @param account   帐户
 * @param authType  认证类型（可传入一种或多种）
 * @param timeout   设定超时时间（默认 -1）
 */
void DeepinAuthFramework::StartAuthentication(const QString &account, const int authType, const int timeout)
{
    if (!m_authenticateControllers->contains(account)) {
        return;
    }
    int ret = m_authenticateControllers->value(account)->Start(authType, timeout);
    if (ret == 0) {
        qInfo() << "Authentication service starts normally." << account << authType;
    }
}

/**
 * @brief 结束本次认证，下次认证前需要先开启认证服务（认证成功或认证失败达到一定次数均会调用此方法）
 *
 * @param account   账户
 * @param authType  认证类型
 */
void DeepinAuthFramework::EndAuthentication(const QString &account, const int authType)
{
    if (!m_authenticateControllers->contains(account)) {
        return;
    }
    qInfo() << "End Authentication:" << account << authType;
    m_authenticateControllers->value(account)->End(authType);
}

/**
 * @brief 将密文发送给认证服务 -- PAM
 *
 * @param account   账户
 * @param authType  认证类型
 * @param token     密文
 */
void DeepinAuthFramework::SendTokenToAuth(const QString &account, const int authType, const QString &token)
{
    if (!m_authenticateControllers->contains(account)) {
        return;
    }
    qInfo() << "Send token to authentication:" << account << authType;

#if 0 // 加密暂时未使用
    void *handle = nullptr;

    if ((handle = dlopen(OPENSSLNAME, RTLD_NOW)) == nullptr) {
        printf("dlopen - %sn", dlerror());
        exit(-1);
    }

    FUNC_BIO_S_MEM d_BIO_s_mem = (FUNC_BIO_S_MEM)dlsym(handle, "BIO_s_mem");

    FUNC_BIO_NEW d_BIO_new = (FUNC_BIO_NEW)dlsym(handle, "BIO_new");

    FUNC_BIO_PUTS d_BIO_puts = (FUNC_BIO_PUTS)dlsym(handle, "BIO_puts");

    PEM_READ_BIO_RSAPUBLICKEY d_PEM_read_bio_RSAPublicKey = (PEM_READ_BIO_RSAPUBLICKEY)dlsym(handle, "PEM_read_bio_RSAPublicKey");

    PEM_READ_BIO_RSA_PUBKEY d_PEM_read_bio_RSA_PUBKEY = (PEM_READ_BIO_RSA_PUBKEY)dlsym(handle, "PEM_read_bio_RSA_PUBKEY");

    RSA_PUBLIC_ENCRYPT d_RSA_public_encrypt = (RSA_PUBLIC_ENCRYPT)dlsym(handle, "RSA_public_encrypt");

    //FUNC_RSA_SIZE d_RSA_size = (FUNC_RSA_SIZE)dlsym(handle, "RSA_size");

    FUNC_RSA_FREE d_RSA_free = (FUNC_RSA_FREE)dlsym(handle, "RSA_free");

    FUNC_RSA_FREE d_BIO_free = (FUNC_RSA_FREE)dlsym(handle, "BIO_free");

    void *bio = d_BIO_new(d_BIO_s_mem());
    if (nullptr == bio)
        qDebug() << "==========bio is null";

    while (!QString(m_pubkey).startsWith("-----BEGIN")) {
        qDebug() << "m_pubkey is error pubkey:" << m_pubkey;
        initPublicKey(account);
    }

    const char *pubKey = m_pubkey;

    d_BIO_puts(bio, pubKey);

    d_RSA d_rsa = nullptr;

    if (0 == strncmp(pubKey, PKCS8_HEADER, strlen(PKCS8_HEADER))) {
        d_rsa = d_PEM_read_bio_RSA_PUBKEY(bio, nullptr, nullptr, nullptr);
    } else if (0 == strncmp(pubKey, PKCS1_HEADER, strlen(PKCS1_HEADER))) {
        d_rsa = d_PEM_read_bio_RSAPublicKey(bio, nullptr, nullptr, nullptr);
    }

    if (d_rsa == nullptr) {
        qWarning() << "d_rsa is nullptr. pubkey:\n"
                   << pubKey;
        return;
    }
    char dData[128];
    d_RSA_public_encrypt(token.length(), (unsigned char *)token.toLatin1().data(), (unsigned char *)dData, d_rsa, 1);

    d_RSA_free(d_rsa);
    d_BIO_free(bio);
    dlclose(handle);

    authControllerInter->SetToken(authType, QByteArray(dData, 128));
#else
    m_authenticateControllers->value(account)->SetToken(authType, token);
#endif
}

/**
 * @brief 设置认证服务退出的方式。
 * AutoQuit: 调用 End 并传入 -1,即可自动退出认证服务；
 * ManualQuit: 调用 End 结束本次认证后，手动调用 Quit 才能退出认证服务。
 * @param flag
 */
void DeepinAuthFramework::SetAuthQuitFlag(const QString &account, const int flag)
{
    if (!m_authenticateControllers->contains(account)) {
        return;
    }
    m_authenticateControllers->value(account)->SetQuitFlag(flag);
}

/**
 * @brief 支持的多因子类型
 *
 * @return int
 */
int DeepinAuthFramework::GetSupportedMixAuthFlags() const
{
    return m_authenticateInter->supportedFlags();
}

/**
 * @brief 一键登录相关
 *
 * @param flag
 * @return QString
 */
QString DeepinAuthFramework::GetPreOneKeyLogin(const int flag) const
{
    return m_authenticateInter->PreOneKeyLogin(flag);
}

/**
 * @brief 获取认证框架类型
 *
 * @return int
 */
int DeepinAuthFramework::GetFrameworkState() const
{
    return m_authenticateInter->frameworkState();
}

/**
 * @brief 支持的加密类型
 *
 * @return QString 加密类型
 */
QString DeepinAuthFramework::GetSupportedEncrypts() const
{
    return m_authenticateInter->supportEncrypts();
}

/**
 * @brief 获取账户被限制时间
 *
 * @param account   账户
 * @return QString  时间
 */
QString DeepinAuthFramework::GetLimitedInfo(const QString &account) const
{
    return m_authenticateInter->GetLimits(account);
}

/**
 * @brief 获取多因子标志位，用于判断是多因子还是单因子认证。
 *
 * @param account   账户
 * @return int
 */
int DeepinAuthFramework::GetMFAFlag(const QString &account) const
{
    if (!m_authenticateControllers->contains(account)) {
        return 0;
    }
    return m_authenticateControllers->value(account)->isMFA();
}

/**
 * @brief 获取 PIN 码的最大长度，用于限制输入的 PIN 码长度。
 *
 * @param account
 * @return int
 */
int DeepinAuthFramework::GetPINLen(const QString &account) const
{
    if (!m_authenticateControllers->contains(account)) {
        return 0;
    }
    return m_authenticateControllers->value(account)->pINLen();
}

/**
 * @brief 模糊多因子信息，供 PAM 使用，这里暂时用不上
 *
 * @param account   账户
 * @return int
 */
int DeepinAuthFramework::GetFuzzyMFA(const QString &account) const
{
    if (!m_authenticateControllers->contains(account)) {
        return 0;
    }
    return m_authenticateControllers->value(account)->isFuzzyMFA();
}

/**
 * @brief 获取总的提示信息，后续可能会融合进 status。
 *
 * @param account
 * @return QString
 */
QString DeepinAuthFramework::GetPrompt(const QString &account) const
{
    if (!m_authenticateControllers->contains(account)) {
        return QString();
    }
    return m_authenticateControllers->value(account)->prompt();
}

/**
 * @brief 获取多因子信息，返回结构体数组，包含多因子认证所有信息。
 *
 * @param account
 * @return MFAInfoList
 */
MFAInfoList DeepinAuthFramework::GetFactorsInfo(const QString &account) const
{
    if (!m_authenticateControllers->contains(account)) {
        return MFAInfoList();
    }
    return m_authenticateControllers->value(account)->factorsInfo();
}

/**
 * @brief 获取认证服务的路径
 *
 * @param account  账户
 * @return QString 认证服务路径
 */
QString DeepinAuthFramework::AuthSessionPath(const QString &account) const
{
    if (!m_authenticateControllers->contains(account)) {
        return QString();
    }
    return m_authenticateControllers->value(account)->path();
}

void DeepinAuthFramework::cancelAuthenticate()
{
    //    if (!m_authagent.isNull()) {
    //        if (m_pamAuth != 0) {
    //            //先取消上次验证请求
    //            m_authagent->setCancelAuth(true);
    //            pthread_cancel(m_pamAuth);
    //            pthread_join(m_pamAuth, nullptr);
    //            m_pamAuth = 0;
    //        }
    //    }
}
