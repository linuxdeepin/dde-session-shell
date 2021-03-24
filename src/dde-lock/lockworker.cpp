#include "lockworker.h"

#include "sessionbasemodel.h"
#include "userinfo.h"

#include <DSysInfo>

#include <QApplication>
#include <QDebug>
#include <QProcess>
#include <QRegularExpression>

#include <grp.h>
#include <libintl.h>
#include <pwd.h>
#include <unistd.h>

#define DOMAIN_BASE_UID 10000

using namespace Auth;
DCORE_USE_NAMESPACE

LockWorker::LockWorker(SessionBaseModel *const model, QObject *parent)
    : AuthInterface(model, parent)
    , m_authenticating(false)
    , m_isThumbAuth(false)
    , m_authFramework(new DeepinAuthFramework(this, this))
    , m_lockInter(new DBusLockService("com.deepin.dde.LockService", "/com/deepin/dde/LockService", QDBusConnection::systemBus(), this))
    , m_hotZoneInter(new DBusHotzone("com.deepin.daemon.Zone", "/com/deepin/daemon/Zone", QDBusConnection::sessionBus(), this))
    , m_sessionManager(new SessionManager("com.deepin.SessionManager", "/com/deepin/SessionManager", QDBusConnection::sessionBus(), this))
    , m_switchosInterface(new HuaWeiSwitchOSInterface("com.huawei", "/com/huawei/switchos", QDBusConnection::sessionBus(), this))
    , m_accountsInter(new AccountsInter("com.deepin.daemon.Accounts", "/com/deepin/daemon/Accounts", QDBusConnection::systemBus(), this))
    , m_loginedInter(new LoginedInter("com.deepin.daemon.Accounts", "/com/deepin/daemon/Logined", QDBusConnection::systemBus(), this))
{
    /* com.deepin.daemon.Accounts */
    m_accountsInter->setSync(false);
    m_model->updateUserList(m_accountsInter->userList());
    m_model->updateLoginedUserList(m_loginedInter->userList());
    /* com.deepin.daemon.Authenticate */
    m_model->updateFrameworkState(m_authFramework->GetFrameworkState());
    m_model->updateSupportedEncryptionType(m_authFramework->GetSupportedEncrypts());
    m_model->updateSupportedMixAuthFlags(m_authFramework->GetSupportedMixAuthFlags());

    m_currentUserUid = getuid();
    m_sessionManager->setSync(false);

    initConnections();

    const bool &LockNoPasswordValue = valueByQSettings<bool>("", "lockNoPassword", false);
    m_model->setIsLockNoPassword(LockNoPasswordValue);

    const QString &switchUserButtonValue = valueByQSettings<QString>("Lock", "showSwitchUserButton", "ondemand");
    m_model->setAlwaysShowUserSwitchButton(switchUserButtonValue == "always");
    m_model->setAllowShowUserSwitchButton(switchUserButtonValue == "ondemand");

    {
        initDBus();
        initData();
    }

    // init ADDomain User
    if (DSysInfo::deepinType() == DSysInfo::DeepinServer || valueByQSettings<bool>("", "loginPromptInput", false)) {
        std::shared_ptr<User> user = std::make_shared<ADDomainUser>(INT_MAX);
        static_cast<ADDomainUser *>(user.get())->setUserDisplayName("...");
        static_cast<ADDomainUser *>(user.get())->setIsServerUser(true);
        m_model->setIsServerModel(true);
        m_model->userAdd(user);
    }
    onUserAdded(ACCOUNTS_DBUS_PREFIX + QString::number(m_currentUserUid));
}

LockWorker::~LockWorker()
{
}

/**
 * @brief 初始化信号连接
 */
void LockWorker::initConnections()
{
    /* com.deepin.daemon.Accounts */
    connect(m_accountsInter, &AccountsInter::UserAdded, m_model, &SessionBaseModel::addUser);
    connect(m_accountsInter, &AccountsInter::UserDeleted, m_model, &SessionBaseModel::removeUser);
    connect(m_accountsInter, &AccountsInter::UserListChanged, m_model, &SessionBaseModel::updateUserList);
    connect(m_loginedInter, &LoginedInter::LastLogoutUserChanged, m_model, &SessionBaseModel::updateLastLogoutUser);
    connect(m_loginedInter, &LoginedInter::UserListChanged, m_model, &SessionBaseModel::updateLoginedUserList);
    /* com.deepin.daemon.Authenticate */
    connect(m_authFramework, &DeepinAuthFramework::FramworkStateChanged, m_model, &SessionBaseModel::updateFrameworkState);
    connect(m_authFramework, &DeepinAuthFramework::LimitedInfoChanged, m_model, &SessionBaseModel::updateLimitedInfo);
    connect(m_authFramework, &DeepinAuthFramework::SupportedEncryptsChanged, m_model, &SessionBaseModel::updateSupportedEncryptionType);
    connect(m_authFramework, &DeepinAuthFramework::SupportedMixAuthFlagsChanged, m_model, &SessionBaseModel::updateSupportedMixAuthFlags);
    /* com.deepin.daemon.Authenticate.Session */
    connect(m_authFramework, &DeepinAuthFramework::FuzzyMFAChanged, m_model, &SessionBaseModel::updateFuzzyMFA);
    connect(m_authFramework, &DeepinAuthFramework::MFAFlagChanged, m_model, &SessionBaseModel::updateMFAFlag);
    connect(m_authFramework, &DeepinAuthFramework::PromptChanged, m_model, &SessionBaseModel::updatePrompt);
    connect(m_authFramework, &DeepinAuthFramework::AuthStatusChanged, m_model, &SessionBaseModel::updateAuthStatus);
    connect(m_authFramework, &DeepinAuthFramework::FactorsInfoChanged, m_model, &SessionBaseModel::updateFactorsInfo);
    /* com.deepin.dde.lock */
    connect(m_lockInter, &DBusLockService::UserChanged, this, &LockWorker::onCurrentUserChanged);
    connect(m_lockInter, &DBusLockService::Event, this, &LockWorker::lockServiceEvent);
    /* com.deepin.SessionManager */
    connect(m_sessionManager, &SessionManager::Unlock, this, [=] {
        m_authenticating = false;
        m_password.clear();
        emit m_model->authFinished(true);
    });
    /* org.freedesktop.login1.Session */
    connect(m_login1SessionSelf, &Login1SessionSelf::ActiveChanged, this, [=](bool active) {
        if (active) {
            m_authenticating = false;
            // m_authFramework->AuthenticateByUser(m_model->currentUser());
        }
    });
    /* org.freedesktop.login1.Manager */
    connect(m_login1Inter, &DBusLogin1Manager::PrepareForSleep, m_model, &SessionBaseModel::prepareForSleep);
    /* model */
    connect(m_model, &SessionBaseModel::onPowerActionChanged, this, &LockWorker::doPowerAction);
    connect(m_model, &SessionBaseModel::lockLimitFinished, this, [=] {
        auto user = m_model->currentUser();
        if (user != nullptr && !user->isLock()) {
            m_password.clear();
            // m_authFramework->AuthenticateByUser(user);
        }
    });
    connect(m_model, &SessionBaseModel::visibleChanged, this, [=](bool visible) {
        std::shared_ptr<User> user_ptr = m_model->currentUser();
        if (visible && user_ptr.get() && user_ptr->type() == User::Native) {
            createAuthentication(user_ptr->name());
            startAuthentication(user_ptr->name(), m_model->getAuthProperty().AuthType);
        }
        if (!visible && user_ptr.get() && user_ptr->type() == User::Native) {
            destoryAuthentication(user_ptr->name());
        }
    });
    connect(m_model, &SessionBaseModel::onStatusChanged, this, [=](SessionBaseModel::ModeStatus status) {
        if (status == SessionBaseModel::ModeStatus::PowerMode || status == SessionBaseModel::ModeStatus::ShutDownMode) {
            checkPowerInfo();
        }
    });
}

void LockWorker::doPowerAction(const SessionBaseModel::PowerAction action)
{
    switch (action) {
    case SessionBaseModel::PowerAction::RequireSuspend:
        m_model->setIsBlackModel(true);
        m_model->setCurrentModeState(SessionBaseModel::ModeStatus::PasswordMode);
        QTimer::singleShot(0, this, [=] {
            m_sessionManager->RequestSuspend();
        });
        break;
    case SessionBaseModel::PowerAction::RequireHibernate:
        m_model->setIsBlackModel(true);
        m_model->setCurrentModeState(SessionBaseModel::ModeStatus::PasswordMode);
        QTimer::singleShot(0, this, [=] {
            m_sessionManager->RequestHibernate();
        });
        break;
    case SessionBaseModel::PowerAction::RequireRestart:
        if (m_model->currentModeState() == SessionBaseModel::ModeStatus::ShutDownMode) {
            m_sessionManager->RequestReboot();
        } else {
            m_model->setCurrentModeState(SessionBaseModel::ModeStatus::ConfirmPasswordMode);
        }
        return;
    case SessionBaseModel::PowerAction::RequireShutdown:
        if (m_model->currentModeState() == SessionBaseModel::ModeStatus::ShutDownMode) {
            m_sessionManager->RequestShutdown();
        } else {
            m_model->setCurrentModeState(SessionBaseModel::ModeStatus::ConfirmPasswordMode);
        }
        return;
    case SessionBaseModel::PowerAction::RequireLock:
        m_model->setLocked(true);
        m_model->setCurrentModeState(SessionBaseModel::ModeStatus::PasswordMode);
        // m_authFramework->AuthenticateByUser(m_model->currentUser());
        break;
    case SessionBaseModel::PowerAction::RequireLogout:
        m_sessionManager->RequestLogout();
        return;
    case SessionBaseModel::PowerAction::RequireSwitchSystem:
        m_switchosInterface->setOsFlag(!m_switchosInterface->getOsFlag());
        QTimer::singleShot(200, this, [=] { m_sessionManager->RequestReboot(); });
        break;
    case SessionBaseModel::PowerAction::RequireSwitchUser:
        m_model->setCurrentModeState(SessionBaseModel::ModeStatus::UserMode);
        // m_authFramework->AuthenticateByUser(m_model->currentUser());
        break;
    default:
        break;
    }

    m_model->setPowerAction(SessionBaseModel::PowerAction::None);
}

void LockWorker::switchToUser(std::shared_ptr<User> user)
{
    // clear old password
    m_password.clear();
    m_authenticating = false;

    // if type is lock, switch to greeter
    QJsonObject json;
    json["Uid"] = static_cast<int>(user->uid());
    json["Type"] = user->type();

    qInfo() << "switch user from" << m_model->currentUser()->name() << " to " << user->name() << json << user->isLogin();

    m_lockInter->SwitchToUser(QString(QJsonDocument(json).toJson(QJsonDocument::Compact))).waitForFinished();

    if (user->isLogin()) {
        QProcess::startDetached("dde-switchtogreeter", QStringList() << user->name());
    } else {
        QProcess::startDetached("dde-switchtogreeter", QStringList());
    }
}

void LockWorker::authUser(const QString &password)
{
    QString userPath = m_accountsInter->FindUserByName(m_model->currentUser()->name());
    if (!userPath.startsWith("/")) {
        qWarning() << userPath;
        onDisplayErrorMsg(userPath);
        return;
    }
    m_authFramework->Authenticate(m_model->currentUser()->name());
    m_authFramework->Responsed(password);
}

void LockWorker::enableZoneDetected(bool disable)
{
    m_hotZoneInter->EnableZoneDetected(disable);
}

/**
 * @brief 显示错误信息
 *
 * @param msg
 */
void LockWorker::onDisplayErrorMsg(const QString &msg)
{
    emit m_model->authFaildTipsMessage(msg);
}

/**
 * @brief 显示提示文案
 *
 * @param msg
 */
void LockWorker::onDisplayTextInfo(const QString &msg)
{
    m_authenticating = false;
    emit m_model->authFaildMessage(msg);
}

/**
 * @brief 显示认证结果
 *
 * @param msg
 */
void LockWorker::onPasswordResult(const QString &msg)
{
    onUnlockFinished(!msg.isEmpty());

    if (msg.isEmpty()) {
        // m_authFramework->AuthenticateByUser(m_model->currentUser());
    }
}

/**
 * @brief 创建认证服务
 * 有用户时，通过dbus发过来的user信息创建认证服务，类服务器模式下通过用户输入的用户创建认证服务
 * @param account
 */
void LockWorker::createAuthentication(const QString &account)
{
    QString userPath = m_accountsInter->FindUserByName(account);
    if (!userPath.startsWith("/")) {
        qWarning() << userPath;
        return;
    }
    switch (m_model->getAuthProperty().FrameworkState) {
    case 0:
        m_authFramework->CreateAuthController(account, m_authFramework->GetSupportedMixAuthFlags(), 0);
        m_model->updateLimitedInfo(m_authFramework->GetLimitedInfo(account));
        m_model->updatePrompt(m_authFramework->GetPrompt(account));
        m_model->updateFactorsInfo(m_authFramework->GetFactorsInfo(account));
        break;
    default:
        m_authFramework->Authenticate(account);
        break;
    }
}

/**
 * @brief 退出认证服务
 *
 * @param account
 */
void LockWorker::destoryAuthentication(const QString &account)
{
    m_authFramework->DestoryAuthController(account);
}

/**
 * @brief 开启认证服务    -- 作为接口提供给上层，屏蔽底层细节
 *
 * @param account   账户
 * @param authType  认证类型（可传入一种或多种）
 * @param timeout   设定超时时间（默认 -1）
 */
void LockWorker::startAuthentication(const QString &account, const int authType)
{
    m_authFramework->StartAuthentication(account, authType, -1);
}

/**
 * @brief 将密文发送给认证服务
 *
 * @param account   账户
 * @param authType  认证类型
 * @param token     密文
 */
void LockWorker::sendTokenToAuth(const QString &account, const int authType, const QString &token)
{
    switch (m_model->getAuthProperty().FrameworkState) {
    case 0:
        m_authFramework->SendTokenToAuth(account, authType, token);
        break;
    default:
        m_authFramework->Responsed(token);
        break;
    }
}

/**
 * @brief 结束本次认证，下次认证前需要先开启认证服务
 *
 * @param account   账户
 * @param authType  认证类型
 */
void LockWorker::endAuthentication(const QString &account, const int authType)
{
    m_authFramework->EndAuthenticatioin(account, authType);
}

void LockWorker::onUserAdded(const QString &user)
{
    std::shared_ptr<User> user_ptr = nullptr;
    uid_t uid = user.mid(QString(ACCOUNTS_DBUS_PREFIX).size()).toUInt();
    if (uid < DOMAIN_BASE_UID) {
        user_ptr = std::make_shared<NativeUser>(user);
    } else {
        user_ptr = std::make_shared<ADDomainUser>(uid);
        qobject_cast<ADDomainUser *>(user_ptr.get())->setUserName(userPwdName(uid));
    }

    if (!user_ptr->isUserIsvalid())
        return;

    user_ptr->setisLogind(isLogined(user_ptr->uid()));

    if (user_ptr->uid() == m_currentUserUid) {
        m_model->setCurrentUser(user_ptr);
    }

    if (user_ptr->uid() == m_lastLogoutUid) {
        m_model->setLastLogoutUser(user_ptr);
    }

    m_model->userAdd(user_ptr);
}

void LockWorker::lockServiceEvent(quint32 eventType, quint32 pid, const QString &username, const QString &message)
{
    if (!m_model->currentUser())
        return;

    if (username != m_model->currentUser()->name())
        return;

    // Don't show password prompt from standard pam modules since
    // we'll provide our own prompt or just not.
    const QString msg = message.simplified() == "Password:" ? "" : message;

    m_authenticating = false;

    if (msg == "Verification timed out") {
        m_isThumbAuth = true;
        emit m_model->authFaildMessage(tr("Fingerprint verification timed out, please enter your password manually"));
        return;
    }

    switch (eventType) {
    case DBusLockService::PromptQuestion:
        qWarning() << "prompt quesiton from pam: " << message;
        emit m_model->authFaildMessage(message);
        break;
    case DBusLockService::PromptSecret:
        qWarning() << "prompt secret from pam: " << message;
        if (m_isThumbAuth && !msg.isEmpty()) {
            emit m_model->authFaildMessage(msg);
        }
        break;
    case DBusLockService::ErrorMsg:
        qWarning() << "error message from pam: " << message;
        if (msg == "Failed to match fingerprint") {
            emit m_model->authFaildTipsMessage(tr("Failed to match fingerprint"));
            emit m_model->authFaildMessage("");
        }
        break;
    case DBusLockService::TextInfo:
        emit m_model->authFaildMessage(QString(dgettext("fprintd", message.toLatin1())));
        break;
    case DBusLockService::Failure:
        onUnlockFinished(false);
        break;
    case DBusLockService::Success:
        onUnlockFinished(true);
        break;
    default:
        break;
    }
}

void LockWorker::onUnlockFinished(bool unlocked)
{
    qWarning() << "LockWorker::onUnlockFinished -- unlocked status : " << unlocked;

    m_authenticating = false;

    //To Do: 最好的方案是修改同步后端认证信息的代码设计
    if (m_model->currentModeState() == SessionBaseModel::ModeStatus::UserMode)
        m_model->setCurrentModeState(SessionBaseModel::ModeStatus::PasswordMode);

    //    if (!unlocked && m_authFramework->GetAuthType() == AuthFlag::Password) {
    //        qWarning() << "Authorization password failed!";
    //        emit m_model->authFaildTipsMessage(tr("Wrong Password"));
    //        return;
    //    }

    switch (m_model->powerAction()) {
    case SessionBaseModel::PowerAction::RequireRestart:
        if (unlocked) {
            m_sessionManager->RequestReboot();
        }
        break;
    case SessionBaseModel::PowerAction::RequireShutdown:
        if (unlocked) {
            m_sessionManager->RequestShutdown();
        }
        break;
    default:
        break;
    }

    emit m_model->authFinished(unlocked);
}

/**
 * @brief LockWorker::onCurrentUserChanged
 * 用来处理初始化切换用户(锁屏+锁屏)或者切换用户(锁屏+登陆)两种种场景的指纹认证
 * @param user
 */
void LockWorker::onCurrentUserChanged(const QString &user)
{
    qWarning() << "LockWorker::onCurrentUserChanged -- change to :" << user;
    if (m_currentUserUid != user)
        m_authFramework->cancelAuthenticate();

    updateLockLimit(m_model->findUserByName(user));
    std::shared_ptr<User> user_ptr = m_model->currentUser();
    const QJsonObject userObj = QJsonDocument::fromJson(user.toUtf8()).object();
    uint uid = static_cast<uint>(userObj["Uid"].toInt());
    if (uid == user_ptr->uid()) {
        createAuthentication(user_ptr->name());
        startAuthentication(user_ptr->name(), m_model->getAuthProperty().AuthType);
    }
    emit m_model->switchUserFinished();
}
