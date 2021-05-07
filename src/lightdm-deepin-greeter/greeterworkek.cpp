#include "greeterworkek.h"
#include "userinfo.h"
#include "keyboardmonitor.h"

#include <libintl.h>
#include <DSysInfo>

#define LOCKSERVICE_PATH "/com/deepin/dde/LockService"
#define LOCKSERVICE_NAME "com.deepin.dde.LockService"

using namespace Auth;
DCORE_USE_NAMESPACE

const QString AuthenticateService("com.deepin.daemon.Authenticate");

class UserNumlockSettings
{
public:
    UserNumlockSettings(const QString &username)
        : m_username(username)
        , m_settings(QSettings::UserScope, "deepin", "greeter")
    {
    }

    bool get(const bool defaultValue) { return m_settings.value(m_username, defaultValue).toBool(); }
    void set(const bool value) { m_settings.setValue(m_username, value); }

private:
    QString m_username;
    QSettings m_settings;
};

GreeterWorkek::GreeterWorkek(SessionBaseModel *const model, QObject *parent)
    : AuthInterface(model, parent)
    , m_greeter(new QLightDM::Greeter(this))
    , m_authFramework(new DeepinAuthFramework(this, this))
    , m_lockInter(new DBusLockService(LOCKSERVICE_NAME, LOCKSERVICE_PATH, QDBusConnection::systemBus(), this))
    , m_authenticating(false)
{
    if (!isConnectSync()) {
        qWarning() << "greeter connect fail !!!";
    }

    m_model->updateFrameworkState(m_authFramework->GetFrameworkState());
    m_model->updateSupportedEncryptionType(m_authFramework->GetSupportedEncrypts());
    m_model->updateSupportedMixAuthFlags(m_authFramework->GetSupportedMixAuthFlags());

    initConnections();

    const QString &switchUserButtonValue {valueByQSettings<QString>("Lock", "showSwitchUserButton", "ondemand")};
    m_model->setAlwaysShowUserSwitchButton(switchUserButtonValue == "always");
    m_model->setAllowShowUserSwitchButton(switchUserButtonValue == "ondemand");

    {
        initDBus();
        initData();

        if (QFile::exists("/etc/deepin/no_suspend"))
            m_model->setCanSleep(false);

        checkDBusServer(m_accountsInter->isValid());
    }

    if (DSysInfo::deepinType() == DSysInfo::DeepinServer || valueByQSettings<bool>("", "loginPromptInput", false)) {
        std::shared_ptr<User> user = std::make_shared<ADDomainUser>(INT_MAX);
        static_cast<ADDomainUser *>(user.get())->setUserDisplayName("...");
        static_cast<ADDomainUser *>(user.get())->setIsServerUser(true);
        m_model->setIsServerModel(true);
        m_model->userAdd(user);
        m_model->setCurrentUser(user);
    } else {
        connect(m_login1Inter, &DBusLogin1Manager::SessionRemoved, this, [=] {
            // lockservice sometimes fails to call on olar server
            QDBusPendingReply<QString> replay = m_lockInter->CurrentUser();
            replay.waitForFinished();

            if (!replay.isError()) {
                const QJsonObject obj = QJsonDocument::fromJson(replay.value().toUtf8()).object();
                auto user_ptr = m_model->findUserByUid(static_cast<uint>(obj["Uid"].toInt()));

                m_model->setCurrentUser(user_ptr);
                // userAuthForLightdm(user_ptr);
            }
        });
    }
    m_model->setCurrentUser(m_lockInter->CurrentUser());
}

void GreeterWorkek::initConnections()
{
    /* greeter */
    connect(m_greeter, &QLightDM::Greeter::showPrompt, this, &GreeterWorkek::prompt);
    connect(m_greeter, &QLightDM::Greeter::showMessage, this, &GreeterWorkek::message);
    connect(m_greeter, &QLightDM::Greeter::authenticationComplete, this, &GreeterWorkek::authenticationComplete);
    /* com.deepin.daemon.Authenticate */
    connect(m_authFramework, &DeepinAuthFramework::FramworkStateChanged, m_model, &SessionBaseModel::updateFrameworkState);
    connect(m_authFramework, &DeepinAuthFramework::LimitedInfoChanged, m_model, &SessionBaseModel::updateLimitedInfo);
    connect(m_authFramework, &DeepinAuthFramework::SupportedEncryptsChanged, m_model, &SessionBaseModel::updateSupportedEncryptionType);
    connect(m_authFramework, &DeepinAuthFramework::SupportedMixAuthFlagsChanged, m_model, &SessionBaseModel::updateSupportedMixAuthFlags);
    /* com.deepin.daemon.Authenticate.Session */
    connect(m_authFramework, &DeepinAuthFramework::AuthStatusChanged, this, [=](const int type, const int status, const QString &message) {
        m_model->updateAuthStatus(type, status, message);
        if (type != -1 && status == 0) {
            endAuthentication(m_account, type);
        }
        if (type == -1 && status == 0 && m_model->getAuthProperty().FrameworkState == 0) {
            m_greeter->respond(m_authFramework->AuthSessionPath(m_account)+","+m_password);
        }
    });
    connect(m_authFramework, &DeepinAuthFramework::FactorsInfoChanged, m_model, &SessionBaseModel::updateFactorsInfo);
    connect(m_authFramework, &DeepinAuthFramework::FuzzyMFAChanged, m_model, &SessionBaseModel::updateFuzzyMFA);
    connect(m_authFramework, &DeepinAuthFramework::MFAFlagChanged, m_model, &SessionBaseModel::updateMFAFlag);
    connect(m_authFramework, &DeepinAuthFramework::PromptChanged, m_model, &SessionBaseModel::updatePrompt);
    /* org.freedesktop.login1.Session */
    connect(m_login1SessionSelf, &Login1SessionSelf::ActiveChanged, this, [=](bool active) {
        if (m_account.isEmpty()) {
            return;
        }
        if (active) {
            if (!m_model->isServerModel() && !m_model->currentUser()->isNoPasswdGrp()) {
                createAuthentication(m_account);
            }
            if (!m_model->isServerModel() && m_model->currentUser()->isNoPasswdGrp() && !m_greeter->inAuthentication()) {
                m_greeter->authenticate(m_account);
            }
        } else {
            if (m_greeter->inAuthentication()) {
                m_greeter->cancelAuthentication();
            }
            destoryAuthentication(m_account);
        }
    });
    /* com.deepin.dde.LockService */
    connect(m_lockInter, &DBusLockService::UserChanged, this, [=](const QString &json) {
        destoryAuthentication(m_account);
        m_model->setCurrentUser(json);
        std::shared_ptr<User> user_ptr = m_model->currentUser();
        const QString &account = user_ptr->name();
        createAuthentication(account);
        emit m_model->switchUserFinished();
    });
    /* model */
    connect(m_model, &SessionBaseModel::authTypeChanged, this, [=](const int type) {
        if (type > 0) {
            startAuthentication(m_account, type);
        }
    });
    connect(m_model, &SessionBaseModel::onPowerActionChanged, this, &GreeterWorkek::doPowerAction);
    connect(m_model, &SessionBaseModel::lockLimitFinished, this, [=] {
        if (!m_greeter->inAuthentication()) {
            m_greeter->authenticate(m_account);
        }
        startAuthentication(m_account, m_model->getAuthProperty().AuthType);
    });
    connect(m_model, &SessionBaseModel::currentUserChanged, this, &GreeterWorkek::recoveryUserKBState);
    connect(m_model, &SessionBaseModel::visibleChanged, this, [=](bool visible) {
        if (visible) {
            if (!m_model->isServerModel() && !m_model->currentUser()->isNoPasswdGrp()) {
                createAuthentication(m_model->currentUser()->name());
            }
            if (!m_model->isServerModel() && m_model->currentUser()->isNoPasswdGrp() && !m_greeter->inAuthentication()) {
                m_greeter->authenticate(m_model->currentUser()->name());
            }
        }
    });
    /* others */
    connect(KeyboardMonitor::instance(), &KeyboardMonitor::numlockStatusChanged, this, [=](bool on) {
        saveNumlockStatus(m_model->currentUser(), on);
    });
}

void GreeterWorkek::doPowerAction(const SessionBaseModel::PowerAction action)
{
    switch (action) {
    case SessionBaseModel::PowerAction::RequireShutdown:
        m_login1Inter->PowerOff(true);
        break;
    case SessionBaseModel::PowerAction::RequireRestart:
        m_login1Inter->Reboot(true);
        break;
    case SessionBaseModel::PowerAction::RequireSuspend:
        m_login1Inter->Suspend(true);
        break;
    case SessionBaseModel::PowerAction::RequireHibernate:
        m_login1Inter->Hibernate(true);
        break;
    default:
        break;
    }

    m_model->setPowerAction(SessionBaseModel::PowerAction::None);
}

void GreeterWorkek::switchToUser(std::shared_ptr<User> user)
{
    qInfo() << "switch user from" << m_model->currentUser()->name() << " to " << user->name() << user->isLogin();

    // clear old password
    m_password.clear();
    m_authenticating = false;

    if (m_greeter->inAuthentication())
        m_greeter->cancelAuthentication();

    // just switch user
    if (user->isLogin()) {
        // switch to user Xorg
        QProcess::startDetached("dde-switchtogreeter", QStringList() << user->name());
    }

    QJsonObject json;
    json["Uid"] = static_cast<int>(user->uid());
    json["Type"] = user->type();
    m_lockInter->SwitchToUser(QString(QJsonDocument(json).toJson(QJsonDocument::Compact))).waitForFinished();
}

void GreeterWorkek::authUser(const QString &password)
{
    if (m_authenticating)
        return;

    m_authenticating = true;

    // auth interface
    std::shared_ptr<User> user = m_model->currentUser();
    m_password = password;

    qWarning() << "greeter authenticate user: " << m_greeter->authenticationUser() << " current user: " << user->name();
    if (m_greeter->authenticationUser() != user->name()) {
        resetLightdmAuth(user, 100, false);
    } else {
        if (m_greeter->inAuthentication()) {
            // m_authFramework->AuthenticateByUser(user);
            // m_authFramework->Responsed(password);
            // m_greeter->respond(password);
        } else {
            m_greeter->authenticate(user->name());
        }
    }
}

void GreeterWorkek::onUserAdded(const QString &user)
{
    std::shared_ptr<NativeUser> user_ptr(new NativeUser(user));

    if (!user_ptr->isUserIsvalid())
        return;
    user_ptr->setisLogind(isLogined(user_ptr->uid()));

    if (m_model->currentUser().get() == nullptr) {
        if (m_model->userList().isEmpty() || m_model->userList().first()->type() == User::ADDomain) {
            m_model->setCurrentUser(user_ptr);
        }
    }

    if (!user_ptr->isLogin() && user_ptr->uid() == m_currentUserUid && !m_model->isServerModel()) {
        m_model->setCurrentUser(user_ptr);
        // userAuthForLightdm(user_ptr);
    }

    if (user_ptr->uid() == m_lastLogoutUid) {
        m_model->setLastLogoutUser(user_ptr);
    }

    connect(user_ptr->getUserInter(), &UserInter::UserNameChanged, this, [=] {
        if (!user_ptr->isNoPasswdGrp()) {
            updateLockLimit(user_ptr);
        }
    });

    m_model->userAdd(user_ptr);
}

/**
 * @brief 创建认证服务
 * 有用户时，通过dbus发过来的user信息创建认证服务，类服务器模式下通过用户输入的用户创建认证服务
 * @param account
 */
void GreeterWorkek::createAuthentication(const QString &account)
{
    m_account = account;
    if (!m_greeter->inAuthentication()) {
        m_greeter->authenticate(account);
    }
    switch (m_model->getAuthProperty().FrameworkState) {
    case 0:
        m_authFramework->CreateAuthController(account, m_authFramework->GetSupportedMixAuthFlags(), 0);
        m_authFramework->SetAuthQuitFlag(account, DeepinAuthFramework::ManualQuit);
        m_model->updateLimitedInfo(m_authFramework->GetLimitedInfo(account));
        break;
    default:
        break;
    }
}

/**
 * @brief 退出认证服务
 *
 * @param account
 */
void GreeterWorkek::destoryAuthentication(const QString &account)
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
void GreeterWorkek::startAuthentication(const QString &account, const int authType)
{
    switch (m_model->getAuthProperty().FrameworkState) {
    case 0:
        m_authFramework->StartAuthentication(account, authType, -1);
        break;
    default:
        m_greeter->authenticate(account);
        break;
    }
}

/**
 * @brief 将密文发送给认证服务
 *
 * @param account   账户
 * @param authType  认证类型
 * @param token     密文
 */
void GreeterWorkek::sendTokenToAuth(const QString &account, const int authType, const QString &token)
{
    //密码输入类型
    if(authType == 1)
      m_password = token;

    switch (m_model->getAuthProperty().FrameworkState) {
    case 0:
        m_authFramework->SendTokenToAuth(account, authType, token);
        break;
    default:
        m_greeter->respond(token);
        break;
    }
}

/**
 * @brief 结束本次认证，下次认证前需要先开启认证服务
 *
 * @param account   账户
 * @param authType  认证类型
 */
void GreeterWorkek::endAuthentication(const QString &account, const int authType)
{
    m_authFramework->EndAuthentication(account, authType);
}

/**
 * @brief 检查用户输入的账户是否合理
 *
 * @param account
 */
void GreeterWorkek::checkAccount(const QString &account)
{
    QString userPath = m_accountsInter->FindUserByName(account);
    if (!userPath.startsWith("/")) {
        qWarning() << userPath;
        onDisplayErrorMsg(userPath);
        return;
    }
    std::shared_ptr<User> user_ptr = std::make_shared<NativeUser>(userPath);
    m_model->setCurrentUser(user_ptr);
    createAuthentication(account);
}

void GreeterWorkek::checkDBusServer(bool isvalid)
{
    if (isvalid) {
        m_accountsInter->userList();
    } else {
        // FIXME: 我不希望这样做，但是QThread::msleep会导致无限递归
        QTimer::singleShot(300, this, [=] {
            qWarning() << "com.deepin.daemon.Accounts is not start, rechecking!";
            checkDBusServer(m_accountsInter->isValid());
        });
    }
}

void GreeterWorkek::oneKeyLogin()
{
    // 多用户一键登陆
    QDBusPendingCall call = m_authenticateInter->PreOneKeyLogin(AuthFlag::Fingerprint);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, [=] {
        if (!call.isError()) {
            QDBusReply<QString> reply = call.reply();
            qWarning() << "one key Login User Name is : " << reply.value();

            auto user_ptr = m_model->findUserByName(reply.value());
            if (user_ptr.get() != nullptr && reply.isValid()) {
                m_model->setCurrentUser(user_ptr);
                userAuthForLightdm(user_ptr);
            }
        } else {
            qWarning() << "pre one key login: " << call.error().message();
        }

        watcher->deleteLater();
    });
}

void GreeterWorkek::userAuthForLightdm(std::shared_ptr<User> user)
{
    if (user.get() != nullptr && !user->isNoPasswdGrp()) {
        //后端需要大约600ms时间去释放指纹设备
        resetLightdmAuth(user, 100, true);
    }
}

void GreeterWorkek::prompt(QString text, QLightDM::Greeter::PromptType type)
{
    // Don't show password prompt from standard pam modules since
    // we'll provide our own prompt or just not.
    qWarning() << "pam prompt: " << text << type;

    const QString msg = text.simplified() == "Please input Password" ? "" : text;

    switch (type) {
    case QLightDM::Greeter::PromptTypeSecret:
        m_authenticating = false;
        if (msg.isEmpty() && !m_password.isEmpty()) {
            // m_greeter->respond(m_password);
        } else {
            emit m_model->authFaildMessage(text);
        }
        break;
    case QLightDM::Greeter::PromptTypeQuestion:
        emit m_model->authTipsMessage(text);
        break;
    }
}

// TODO(justforlxz): 错误信息应该存放在User类中, 切换用户后其他控件读取错误信息，而不是在这里分发。
void GreeterWorkek::message(QString text, QLightDM::Greeter::MessageType type)
{
    qWarning() << "pam message: " << text << type;

    switch (type) {
    case QLightDM::Greeter::MessageTypeInfo:
        qWarning() << Q_FUNC_INFO << "lightdm greeter message type info: " << text.toUtf8() << QString(dgettext("fprintd", text.toUtf8()));
        emit m_model->authFaildMessage(QString(dgettext("fprintd", text.toUtf8())));
        break;

    case QLightDM::Greeter::MessageTypeError:
        emit m_model->authFaildTipsMessage(QString(dgettext("fprintd", text.toUtf8())));
        break;
    }
}

void GreeterWorkek::authenticationComplete()
{
    qInfo() << "authentication complete, authenticated " << m_greeter->isAuthenticated();

    if (!m_greeter->isAuthenticated()) {
        // m_authenticating = false;
        // if (m_password.isEmpty()) {
        //     resetLightdmAuth(m_model->currentUser(), 100, false);
        //     return;
        // }

        // m_password.clear();

        if (m_model->currentUser()->type() == User::Native) {
            emit m_model->authFaildTipsMessage(tr("Wrong Password"));
        }

        if (m_model->currentUser()->type() == User::ADDomain) {
            emit m_model->authFaildTipsMessage(tr("The account or password is not correct. Please enter again."));
        }

        // resetLightdmAuth(m_model->currentUser(), 100, false);

        return;
    }

    emit m_model->authFinished(m_greeter->isAuthenticated());

    m_password.clear();

    switch (m_model->powerAction()) {
    case SessionBaseModel::PowerAction::RequireRestart:
        m_login1Inter->Reboot(true);
        return;
    case SessionBaseModel::PowerAction::RequireShutdown:
        m_login1Inter->PowerOff(true);
        return;
    default:
        break;
    }

    qWarning() << "start session = " << m_model->sessionKey();

    auto startSessionSync = [=] () {
        QJsonObject json;
        json["Uid"] = static_cast<int>(m_model->currentUser()->uid());
        json["Type"] = m_model->currentUser()->type();
        m_lockInter->SwitchToUser(QString(QJsonDocument(json).toJson(QJsonDocument::Compact))).waitForFinished();

        m_greeter->startSessionSync(m_model->sessionKey());
        m_authenticating = false;
    };

    // NOTE(kirigaya): It is not necessary to display the login animation.
    connect(m_model->currentUser().get(), &User::desktopBackgroundPathChanged, this, &GreeterWorkek::requestUpdateBackground);
    m_model->currentUser()->desktopBackgroundPath();

#ifndef DISABLE_LOGIN_ANI
    QTimer::singleShot(1000, this, startSessionSync);
#else
    startSessionSync();
#endif
    destoryAuthentication(m_account);
}

void GreeterWorkek::saveNumlockStatus(std::shared_ptr<User> user, const bool &on)
{
    UserNumlockSettings(user->name()).set(on);
}

void GreeterWorkek::recoveryUserKBState(std::shared_ptr<User> user)
{
    //FIXME(lxz)
    //    PowerInter powerInter("com.deepin.system.Power", "/com/deepin/system/Power", QDBusConnection::systemBus(), this);
    //    const BatteryPresentInfo info = powerInter.batteryIsPresent();
    //    const bool defaultValue = !info.values().first();
    if (user.get() == nullptr)
        return;

    const bool enabled = UserNumlockSettings(user->name()).get(false);

    qWarning() << "restore numlock status to " << enabled;

    // Resync numlock light with numlock status
    bool cur_numlock = KeyboardMonitor::instance()->isNumlockOn();
    KeyboardMonitor::instance()->setNumlockStatus(!cur_numlock);
    KeyboardMonitor::instance()->setNumlockStatus(cur_numlock);

    KeyboardMonitor::instance()->setNumlockStatus(enabled);
}

//TODO 显示内容
void GreeterWorkek::onDisplayErrorMsg(const QString &msg)
{
    emit m_model->authFaildTipsMessage(msg);
}

void GreeterWorkek::onDisplayTextInfo(const QString &msg)
{
    //m_authenticating = false;
    emit m_model->authFaildMessage(msg);
}

void GreeterWorkek::onPasswordResult(const QString &msg)
{
    //onUnlockFinished(!msg.isEmpty());

    //if(msg.isEmpty()) {
    //    m_authFramework->AuthenticateByUser(m_model->currentUser());
    //}
}

void GreeterWorkek::resetLightdmAuth(std::shared_ptr<User> user, int delay_time, bool is_respond)
{
    if (user->isLock()) {
        return;
    }

    QTimer::singleShot(delay_time, this, [=] {
        // m_greeter->authenticate(user->name());
        // m_authFramework->AuthenticateByUser(user);
        if (is_respond && !m_password.isEmpty()) {
            // if (m_framworkState == 0) {
            //其他
            // } else if (m_framworkState == 1) {
            //没有修改过
            // m_authFramework->Responsed(m_password);
            // } else if (m_framworkState == 2) {
            //修改过的pam
            // m_greeter->respond(m_password);
            // }
        }
    });
}
