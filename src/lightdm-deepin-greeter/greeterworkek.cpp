#include "greeterworkek.h"
#include "src/session-widgets/sessionbasemodel.h"
#include "src/session-widgets/userinfo.h"
#include "src/global_util/keyboardmonitor.h"

#include <libintl.h>
#include <DSysInfo>

#define LOCKSERVICE_PATH "/com/deepin/dde/LockService"
#define LOCKSERVICE_NAME "com.deepin.dde.LockService"

using namespace Auth;
DCORE_USE_NAMESPACE

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
    , m_login1ManagerInterface(new DBusLogin1Manager("org.freedesktop.login1",
                                                     "/org/freedesktop/login1",
                                                     QDBusConnection::systemBus(), this))
    , m_lockInter(new DBusLockService(LOCKSERVICE_NAME, LOCKSERVICE_PATH, QDBusConnection::systemBus(), this))
    , m_isThumbAuth(false)
    , m_authenticating(false)
    , m_password(QString())
{
    m_authFramework = new DeepinAuthFramework(this, this);
    m_authFramework->setAuthType(DeepinAuthFramework::AuthType::LightdmType);

    QObject::connect(model, &SessionBaseModel::onStatusChanged, this, [ = ](SessionBaseModel::ModeStatus state) {
        std::shared_ptr<User> user = m_model->currentUser();
        m_authFramework->SetUser(user);
        if (SessionBaseModel::ModeStatus::PasswordMode == state) {
            //active fprinter auth
            m_authFramework->Authenticate();
        } else {
            //close fprinter auth
            m_authFramework->Clear();
        }
    });

    connect(model, &SessionBaseModel::lockChanged, this, [ = ](bool is_lock) {
        if (is_lock) {
            m_authFramework->Clear();
        } else {
            m_authFramework->Authenticate();
        }
    });

    if (!m_login1ManagerInterface->isValid()) {
        qWarning() << "m_login1ManagerInterface:"
                   << m_login1ManagerInterface->lastError().type();
    }

    if (!m_greeter->connectSync()) {
        qWarning() << "greeter connect fail !!!";
    }

    connect(m_greeter, &QLightDM::Greeter::showPrompt, this, &GreeterWorkek::prompt);
    connect(m_greeter, &QLightDM::Greeter::showMessage, this, &GreeterWorkek::message);
    connect(m_greeter, &QLightDM::Greeter::authenticationComplete, this, &GreeterWorkek::authenticationComplete);

    connect(model, &SessionBaseModel::onPowerActionChanged, this, [ = ](SessionBaseModel::PowerAction poweraction) {
        switch (poweraction) {
        case SessionBaseModel::PowerAction::RequireShutdown:
            m_login1ManagerInterface->PowerOff(true);
            break;
        case SessionBaseModel::PowerAction::RequireRestart:
            m_login1ManagerInterface->Reboot(true);
            break;
        case SessionBaseModel::PowerAction::RequireSuspend:
            m_login1ManagerInterface->Suspend(true);
            break;
        case SessionBaseModel::PowerAction::RequireHibernate:
            m_login1ManagerInterface->Hibernate(true);
            break;
        default:
            break;
        }

        model->setPowerAction(SessionBaseModel::PowerAction::None);
    });

    connect(KeyboardMonitor::instance(), &KeyboardMonitor::numlockStatusChanged, this, [ = ](bool on) {
        saveNumlockStatus(model->currentUser(), on);
    });

    connect(model, &SessionBaseModel::currentUserChanged, this, &GreeterWorkek::recoveryUserKBState);
    connect(m_lockInter, &DBusLockService::UserChanged, this, &GreeterWorkek::onCurrentUserChanged);

    const QString &switchUserButtonValue { valueByQSettings<QString>("Lock", "showSwitchUserButton", "ondemand") };
    m_model->setAlwaysShowUserSwitchButton(switchUserButtonValue == "always");
    m_model->setAllowShowUserSwitchButton(switchUserButtonValue == "ondemand");

    {
        initDBus();
        initData();

        if (QFile::exists("/etc/deepin/no_suspend"))
            m_model->setCanSleep(false);

        checkDBusServer(m_accountsInter->isValid());
        onCurrentUserChanged(m_lockInter->CurrentUser());
    }

    if (DSysInfo::deepinType() == DSysInfo::DeepinServer || valueByQSettings<bool>("", "loginPromptInput", false)) {
        std::shared_ptr<User> user = std::make_shared<ADDomainUser>(0);
        static_cast<ADDomainUser *>(user.get())->setUserDisplayName("...");
        static_cast<ADDomainUser *>(user.get())->setIsServerUser(true);
        m_model->setIsServerModel(true);
        m_model->userAdd(user);
        m_model->setCurrentUser(user);
    }
}

void GreeterWorkek::switchToUser(std::shared_ptr<User> user)
{
    qDebug() << "switch user from" << m_model->currentUser()->name() << " to "
             << user->name();

    // clear old password
    m_password.clear();
    m_authenticating = false;

    // just switch user
    if (user->isLogin()) {
        // switch to user Xorg
        QProcess::startDetached("dde-switchtogreeter", QStringList() << user->name());
    }

    if (isDeepin()) {
        m_authFramework->Clear();
        m_authFramework->SetUser(user);
        m_authFramework->Authenticate();
    }

    QJsonObject json;
    json["Uid"] = static_cast<int>(user->uid());
    json["Type"] = user->type();
    m_lockInter->SwitchToUser(QString(QJsonDocument(json).toJson(QJsonDocument::Compact))).waitForFinished();
    m_greeter->cancelAuthentication();

    //防止切换后用户后dbus没有发送UserChanged
    onCurrentUserChanged(QJsonDocument(json).toJson());
}

void GreeterWorkek::authUser(const QString &password)
{
    if (m_authenticating) return;

    m_authenticating = true;

    // auth interface
    std::shared_ptr<User> user = m_model->currentUser();

    m_password = password;

    qDebug() << "start authentication of user: " << user->name();

    if (isDeepin() && !user->isNoPasswdGrp()) {
        m_authFramework->Clear();
        m_authFramework->SetUser(user);
        m_authFramework->setPassword(m_password);
        m_authFramework->Authenticate();
        return;
    }

    greeterAuthUser(password);
}

void GreeterWorkek::greeterAuthUser(const QString &password)
{
    std::shared_ptr<User> user = m_model->currentUser();
    m_password = password;

    if (m_greeter->authenticationUser() != user->name()) {
        m_greeter->cancelAuthentication();
        QTimer::singleShot(100, this, [ = ] { m_greeter->authenticate(user->name()); });
    } else {
        if (m_greeter->inAuthentication()) {
            m_greeter->cancelAuthentication();
        }
        m_greeter->authenticate(user->name());
    }
}

void GreeterWorkek::onUserAdded(const QString &user)
{
    std::shared_ptr<User> user_ptr(new NativeUser(user));

    user_ptr->setisLogind(isLogined(user_ptr->uid()));

    if (m_model->currentUser().get() == nullptr) {
        if (m_model->userList().isEmpty() ||
                m_model->userList().first()->type() == User::ADDomain) {
            m_model->setCurrentUser(user_ptr);

            if (m_model->currentType() == SessionBaseModel::AuthType::LightdmType) {
                userAuthForLightdm(user_ptr);
            }
        }
    }

    if (user_ptr->uid() == m_lastLogoutUid) {
        m_model->setLastLogoutUser(user_ptr);
    }

    m_model->userAdd(user_ptr);
}

void GreeterWorkek::checkDBusServer(bool isvalid)
{
    if (isvalid) {
        onUserListChanged(m_accountsInter->userList());
    } else {
        // FIXME: 我不希望这样做，但是QThread::msleep会导致无限递归
        QTimer::singleShot(300, this, [ = ] {
            qWarning() << "com.deepin.daemon.Accounts is not start, rechecking!";
            checkDBusServer(m_accountsInter->isValid());
        });
    }
}

void GreeterWorkek::onCurrentUserChanged(const QString &user)
{
    const QJsonObject obj = QJsonDocument::fromJson(user.toUtf8()).object();
    m_currentUserUid = static_cast<uint>(obj["Uid"].toInt());

    for (std::shared_ptr<User> user : m_model->userList()) {
        if (!user->isLogin() && user->uid() == m_currentUserUid) {
            m_model->setCurrentUser(user);
            userAuthForLightdm(user);
            break;
        }
    }
}

void GreeterWorkek::userAuthForLightdm(std::shared_ptr<User> user)
{
    if (isDeepin() && !user->isNoPasswdGrp()) {
        m_authFramework->Clear();
        m_authFramework->SetUser(user);
        m_authFramework->setPassword(m_password);
        m_authFramework->Authenticate();
        return;
    }

    if (!user->isNoPasswdGrp()) {
        if (m_greeter->inAuthentication()) {
            m_greeter->cancelAuthentication();
        }
        QTimer::singleShot(100, this, [ = ] {
            m_greeter->authenticate(user->name());
            m_greeter->respond(m_password);
        });
    }
}

void GreeterWorkek::prompt(QString text, QLightDM::Greeter::PromptType type)
{
    qDebug() << "pam prompt: " << text << type;

    // Don't show password prompt from standard pam modules since
    // we'll provide our own prompt or just not.
    const QString msg = text.simplified() == "Password:" ? "" : text;

    switch (type) {
    case QLightDM::Greeter::PromptTypeSecret:
        if (m_isThumbAuth || m_password.isEmpty()) break;

        if (msg.isEmpty()) {
            m_greeter->respond(m_password);
        } else {
            emit m_model->authFaildMessage(msg);
        }
        break;
    case QLightDM::Greeter::PromptTypeQuestion:
        // trim the right : in the message if exists.
        emit m_model->authFaildMessage(text.replace(":", ""));
        break;
    }
}

// TODO(justforlxz): 错误信息应该存放在User类中, 切换用户后其他控件读取错误信息，而不是在这里分发。
void GreeterWorkek::message(QString text, QLightDM::Greeter::MessageType type)
{
    qDebug() << "pam message: " << text << type;

    if (text == "Verification timed out") {
        m_isThumbAuth = true;

        //V20版本新需求：若用户输入了密码，当指纹解锁超时，自动校验一次密码登录
        if (!m_password.isEmpty()) {
            QTimer::singleShot(300, this, [ = ] {
                m_greeter->respond(m_password);
            });
        }
//        emit m_model->authFaildMessage(tr(
//            "Fingerprint verification timed out, please enter your password manually"));
        return;
    }

    switch (type) {
    case QLightDM::Greeter::MessageTypeInfo:
        if (m_isThumbAuth) break;

        emit m_model->authFaildMessage(QString(dgettext("fprintd", text.toLatin1())));
        break;
    case QLightDM::Greeter::MessageTypeError:
        qWarning() << "error message from lightdm: " << text;
        if (text == "Failed to match fingerprint") {
            emit m_model->authFaildMessage("");
            //V20版本新需求，在指纹解锁失败和超时情况下，不提示任何信息
            //emit m_model->authFaildTipsMessage(tr("Failed to match fingerprint"));
        }
        break;
    }
}

void GreeterWorkek::authenticationComplete()
{
    qDebug() << "authentication complete, authenticated " << m_greeter->isAuthenticated();

    m_authenticating = false;

    if (!m_greeter->isAuthenticated()) {
        if (m_password.isEmpty()) {
            return;
        }

        if (m_model->currentUser()->type() == User::Native) {
            emit m_model->authFaildTipsMessage(tr("Wrong Password"));
        }

        if (m_model->currentUser()->type() == User::ADDomain) {
            emit m_model->authFaildTipsMessage(
                tr("The account or password is not correct. Please enter again."));
        }

        if (m_model->currentUser()->isLockForNum()) {
            m_model->currentUser()->startLock();
        }

        return;
    }

    switch (m_model->powerAction()) {
    case SessionBaseModel::PowerAction::RequireRestart:
        m_login1ManagerInterface->Reboot(true);
        return;
    case SessionBaseModel::PowerAction::RequireShutdown:
        m_login1ManagerInterface->PowerOff(true);
        return;
    default: break;
    }

    qDebug() << "start session = " << m_model->sessionKey();

    auto startSessionSync = [ = ]() {
        QJsonObject json;
        json["Uid"]  = static_cast<int>(m_model->currentUser()->uid());
        json["Type"] = m_model->currentUser()->type();
        m_lockInter->SwitchToUser(QString(QJsonDocument(json).toJson(QJsonDocument::Compact))).waitForFinished();

        m_greeter->startSessionSync(m_model->sessionKey());
    };

    // NOTE(kirigaya): It is not necessary to display the login animation.
    emit requestUpdateBackground(m_model->currentUser()->desktopBackgroundPath());
    emit m_model->authFinished(true);

#ifndef DISABLE_LOGIN_ANI
    QTimer::singleShot(1000, this, startSessionSync);
#else
    startSessionSync();
#endif
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
    const bool enabled = UserNumlockSettings(user->name()).get(false);

    qDebug() << "restore numlock status to " << enabled;

    // Resync numlock light with numlock status
    bool cur_numlock = KeyboardMonitor::instance()->isNumlockOn();
    KeyboardMonitor::instance()->setNumlockStatus(!cur_numlock);
    KeyboardMonitor::instance()->setNumlockStatus(cur_numlock);

    KeyboardMonitor::instance()->setNumlockStatus(enabled);

    m_currentUserUid = user->uid();
    m_authFramework->setCurrentUid(m_currentUserUid);
}

void GreeterWorkek::onDisplayErrorMsg(AuthAgent::Type type, const QString &errtype, const QString &msg)
{
    if (type == AuthAgent::Fprint) {
        if (errtype != "verify-timed-out") {
            emit m_model->authFaildTipsMessage(msg, SessionBaseModel::Fprint);
        } else {
            emit m_model->authFaildMessage("", SessionBaseModel::Fprint);
        }
    } else {
        emit m_model->authFaildMessage(msg);
    }
}

void GreeterWorkek::onDisplayTextInfo(AuthAgent::Type type, const QString &msg)
{
    if (type == AuthAgent::Fprint) {
        emit m_model->authFaildMessage(msg, SessionBaseModel::Fprint);
    } else {
        emit m_model->authFaildMessage(msg);
    }
}

void GreeterWorkek::onPasswordResult(AuthAgent::Type type, const QString &msg)
{
    if (msg.isEmpty()) {
        if (type == AuthAgent::Fprint) {
            qDebug() << Q_FUNC_INFO << "Fprint Failed";
            emit m_model->authFaildMessage("", SessionBaseModel::Fprint);
        } else {
            qDebug() << Q_FUNC_INFO << "Password Failed";
            authenticationComplete();
        }
    } else {
        m_password = msg;

        if (m_model->currentUser()->isPasswordExpired()) {
            m_authenticating = false;
            m_model->setCurrentModeState(SessionBaseModel::ModeStatus::ChangePasswordMode);
        } else {
            qDebug() << Q_FUNC_INFO << msg;
            greeterAuthUser(m_password);

            QTimer::singleShot(5000, this, [ = ](){
                qDebug() << "not receive auth complete signal force start session";
                if(m_greeter->isAuthenticated()) authenticationComplete();
            });
        }
    }
}

