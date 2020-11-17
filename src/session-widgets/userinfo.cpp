#include "userinfo.h"
#include "src/global_util/constants.h"

#include <unistd.h>
#include <pwd.h>
#include <grp.h>

static const std::vector<uint> DEFAULT_WAIT_TIME = {3, 5, 15, 60, 1440};

static QString userPwdName(__uid_t uid)
{
    if (uid < 1000) return QString();

    struct passwd *pw = nullptr;
    /* Fetch passwd structure (contains first group ID for user) */
    pw = getpwuid(uid);

    return QString().fromLocal8Bit(pw->pw_name);
}

static bool checkUserIsNoPWGrp(User const *user)
{
    if (user->type() == User::ADDomain) {
        return false;
    }

    // Caution: 32 here is unreliable, and there may be more
    // than this number of groups that the user joins.

    int ngroups = 32;
    gid_t groups[32];
    struct passwd *pw = nullptr;
    struct group *gr = nullptr;

    /* Fetch passwd structure (contains first group ID for user) */
    pw = getpwnam(user->name().toUtf8().data());
    if (pw == nullptr) {
        qDebug() << "fetch passwd structure failed, username: " << user->name();
        return false;
    }

    /* Retrieve group list */

    if (getgrouplist(user->name().toUtf8().data(), pw->pw_gid, groups, &ngroups) == -1) {
        fprintf(stderr, "getgrouplist() returned -1; ngroups = %d\n",
                ngroups);
        return false;
    }

    /* Display list of retrieved groups, along with group names */

    for (int i = 0; i < ngroups; i++) {
        gr = getgrgid(groups[i]);
        if (gr != nullptr && QString(gr->gr_name) == QString("nopasswdlogin")) {
            return true;
        }
    }

    return false;
}

static const QString toLocalFile(const QString &path)
{
    QUrl url(path);

    if (url.isLocalFile()) {
        return url.path();
    }

    return url.url();
}

User::User(QObject *parent)
    : QObject(parent)
    , m_isLogind(false)
    , m_isLock(false)
    , m_locale(getenv("LANG"))
    , m_lockTimer(new QTimer)
{
    m_lockData.lockTime = 0;
    m_lockData.tryNum = 0;
    auto waitTimeAvail = [ = ](std::vector<uint> *waitTime) {
        if (waitTime->empty())
            return false;

        for (uint i = 0;i < waitTime->size();i++) {
            if (i < waitTime->size() - 1) {
                // 需求规定，等待的最大时间不得超过2880分钟
                if (waitTime->at(i) > 2880)
                    waitTime->at(i) = 2880;
                // 需求规定，等待时间前一次必须小于等于后一次
                if (waitTime->at(i) > waitTime->at(i+1))
                    waitTime->at(i+1) = waitTime->at(i);
            }
        }
        return true;
    };

    // 可配置认证密码失败,且超过失败次数后的间隔限制时间
    QString lockWaitTime = findValueByQSettings<QString>(DDESESSIONCC::session_ui_configs, "LockTime", "lockWaitTime", "Default");
    if(lockWaitTime == "Default")
        m_lockData.waitTime = DEFAULT_WAIT_TIME;
    else {
        std::vector<uint> waitTime;
        for (QString i : lockWaitTime.split(",")) {
            waitTime.push_back(i.toUInt());
        }

        m_lockData.waitTime = waitTimeAvail(&waitTime) ? waitTime : DEFAULT_WAIT_TIME;
    }

    // 可配置进入等待前的失败次数
    uint lockLimitTryNum = findValueByQSettings<uint>(DDESESSIONCC::session_ui_configs, "LockTime", "lockLimitTryNum", 0);
    if(lockLimitTryNum == 0 || lockLimitTryNum > 10) m_lockData.limitTryNum = 5;
    else m_lockData.limitTryNum = lockLimitTryNum;

    m_lockTimer->setInterval(1000 * 60);
    m_lockTimer->setSingleShot(false);
    connect(m_lockTimer.get(), &QTimer::timeout, this, &User::onLockTimeOut);
}

User::User(const User &user)
    : QObject(user.parent())
    , m_isLogind(user.m_isLogind)
    , m_isLock(user.m_isLock)
    , m_uid(user.m_uid)
    , m_lockData(user.m_lockData)
    , m_userName(user.m_userName)
    , m_locale(user.m_locale)
    , m_lockTimer(user.m_lockTimer)
{

}

bool User::operator==(const User &user) const
{
    return type() == user.type() &&
           m_uid == user.m_uid;
}

void User::setLocale(const QString &locale)
{
    if (m_locale == locale) return;

    m_locale = locale;

    emit localeChanged(locale);
}

bool User::isNoPasswdGrp() const
{
    return checkUserIsNoPWGrp(this);
}

bool User::isUserIsvalid() const
{
    return true;
}

void User::setisLogind(bool isLogind)
{
    if (m_isLogind == isLogind) {
        return;
    }

    m_isLogind = isLogind;

    emit logindChanged(isLogind);
}

void User::setPath(const QString &path)
{
    if (m_path == path) return;

    m_path = path;
}

bool User::isLockForNum()
{
    m_lockData.tryNum++;
    return m_isLock || (m_lockData.tryNum >= m_lockData.limitTryNum);
}

void User::startLock()
{
    m_startTime = time(nullptr);//切换到其他用户时，由于Qtimer自身机制导致无法进入timeout事件，导致被锁定的账户不能继续执行，解决bug4511

    if (m_lockTimer->isActive()) return;

    m_isLock = true;

    if ((m_lockData.tryNum - m_lockData.limitTryNum + 1) > m_lockData.waitTime.size())
        m_lockData.lockTime = m_lockData.waitTime.back();
    else
        m_lockData.lockTime = m_lockData.waitTime[m_lockData.tryNum - m_lockData.limitTryNum];

    onLockTimeOut();
}

void User::resetLock()
{
    m_lockData.tryNum = 0;
    m_startTime = 0;
}

void User::onLockTimeOut()
{
    time_t stopTime = time(nullptr);
    auto min = static_cast<uint>((stopTime - m_startTime) / 60);
    uint minLimit = ((m_lockData.tryNum - m_lockData.limitTryNum + 1) > m_lockData.waitTime.size()) ? m_lockData.waitTime.back() : m_lockData.waitTime[m_lockData.tryNum - m_lockData.limitTryNum];
    if (min >= minLimit) {
        // 等到时间到
        m_isLock = false;
        m_lockTimer->stop();
    } else {
        // 未到达等待时间
        m_lockData.lockTime = minLimit - min;
        m_lockTimer->start();
    }

    emit lockChanged(m_isLock);
}

NativeUser::NativeUser(const QString &path, QObject *parent)
    : User(parent)
    , m_userInter(new UserInter(ACCOUNT_DBUS_SERVICE, path, QDBusConnection::systemBus(), this))
{
    m_userInter->setSync(false);
    connect(m_userInter, &UserInter::IconFileChanged, this, [ = ](const QString & avatar) {
        m_avatar = avatar;
        emit avatarChanged(avatar);
    });
    
    connect(m_userInter, &UserInter::FullNameChanged, this, [ = ](const QString & fullname) {
        m_fullName = fullname;
        emit displayNameChanged(fullname.isEmpty() ? m_userName : fullname);
    });

    connect(m_userInter, &UserInter::UserNameChanged, this, [ = ](const QString & user_name) {
        m_userName = user_name;
        emit displayNameChanged(m_fullName.isEmpty() ? m_userName : m_fullName);
    });

    connect(m_userInter, &UserInter::UidChanged, this, [ = ](const QString & uid) {
        m_uid = uid.toUInt();
    });

    connect(m_userInter, &UserInter::DesktopBackgroundsChanged, this, [ = ] (const QStringList & paths) {
        QSettings settings(DDESESSIONCC::CONFIG_FILE + m_userName, QSettings::IniFormat);
        settings.beginGroup("User");

        int index = settings.value("Workspace").toInt();
        if (paths.size() >= index && index > 0) {
            m_desktopBackground = toLocalFile(paths.at(index - 1));
            emit desktopBackgroundPathChanged(m_desktopBackground);
        } else {
            qDebug() << "DesktopBackgroundsChanged get error index:" << index << ", paths:" << paths;
        }
    });

    connect(m_userInter, &UserInter::GreeterBackgroundChanged, this, [ = ](const QString & path) {
        m_greeterBackground = toLocalFile(path);
        emit greeterBackgroundPathChanged(m_greeterBackground);
    });

    connect(m_userInter, &UserInter::PasswordStatusChanged, this, [ = ](const QString& status){
        m_noPasswdGrp = (status == "NP" || checkUserIsNoPWGrp(this));
    });

    //监听用户时制改变信号,并发送信号让界面刷新
    connect(m_userInter, &UserInter::Use24HourFormatChanged, this, [ = ](bool is24HourFormat) {
        m_is24HourFormat = is24HourFormat;
        emit use24HourFormatChanged(m_is24HourFormat);
    });

    connect(m_userInter, &UserInter::LocaleChanged, this, &NativeUser::setLocale);
    connect(m_userInter, &UserInter::HistoryLayoutChanged, this, &NativeUser::kbLayoutListChanged);
    connect(m_userInter, &UserInter::LayoutChanged, this, &NativeUser::currentKBLayoutChanged);
    connect(m_userInter, &UserInter::NoPasswdLoginChanged, this, &NativeUser::noPasswdLoginChanged);

    m_userInter->userName();
    m_userInter->locale();
    m_userInter->passwordStatus();
    m_userInter->fullName();

    // intercept account dbus path uid
    m_uid = path.mid(QString(ACCOUNTS_DBUS_PREFIX).size()).toUInt();
    m_userName = userPwdName(m_uid);
    m_noPasswdGrp = checkUserIsNoPWGrp(this);

    configAccountInfo(DDESESSIONCC::CONFIG_FILE + m_userName);

    QDBusPendingCall pass_expired = m_userInter->IsPasswordExpired();
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pass_expired, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, [ = ] {
         QDBusReply<bool> reply = pass_expired.reply();
        if (!pass_expired.isError() && reply.isValid()) {
            m_isPasswdExpired = reply.value();
        }
        watcher->deleteLater();
    });

    setPath(path);
}

void NativeUser::configAccountInfo(const QString &account_config)
{
    QSettings settings(account_config, QSettings::IniFormat);
    settings.beginGroup("User");

    m_avatar = settings.value("Icon").toString();
    m_greeterBackground = toLocalFile(settings.value("GreeterBackground").toString());
    m_locale = settings.value("Locale").toString();
    //初始化时读取用户的配置文件中设置的时制格式
    m_is24HourFormat = settings.value("Use24HourFormat").toBool();

    QStringList desktopBackgrounds = settings.value("DesktopBackgrounds").toString().split(";");
    int index = settings.value("Workspace").toInt();
    if (desktopBackgrounds.size() >= index && index > 0) {
        m_desktopBackground = toLocalFile(desktopBackgrounds.at(index - 1));
    } else {
        qDebug() << "configAccountInfo get error index:" << index << ", backgrounds:" << desktopBackgrounds;
    }
}

void NativeUser::setCurrentLayout(const QString &layout)
{
    m_userInter->SetLayout(layout);
}

QString NativeUser::displayName() const
{
    return m_fullName.isEmpty() ? m_userName : m_fullName;
}

QString NativeUser::avatarPath() const
{
    m_userInter->iconFile();
    return m_avatar;
}

QString NativeUser::greeterBackgroundPath() const
{
    m_userInter->greeterBackground();
    return m_greeterBackground;
}

QString NativeUser::desktopBackgroundPath() const
{
    m_userInter->desktopBackgrounds();
    return m_desktopBackground;
}

QStringList NativeUser::kbLayoutList()
{
    return m_userInter->historyLayout();
}

QString NativeUser::currentKBLayout()
{
    return m_userInter->layout();
}

bool NativeUser::isNoPasswdGrp() const
{
    return m_noPasswdGrp;
}

bool NativeUser::isPasswordExpired() const
{
    return m_isPasswdExpired;
}

bool NativeUser::isUserIsvalid() const
{
    //无效用户的时候m_userInter是有效的
    return m_userInter->isValid();
}

bool NativeUser::is24HourFormat() const
{
    if(!isUserIsvalid() || m_isServer)
        return true;

    //先触发后端刷新取用户时制信号,并返回当前已经取到的时制格式
    m_userInter->use24HourFormat();
    return m_is24HourFormat;
}

ADDomainUser::ADDomainUser(uid_t uid, QObject *parent)
    : User(parent)
{
    m_uid = uid;
}

void ADDomainUser::setUserDisplayName(const QString &name)
{
    if (m_displayName == name) {
        return;
    }

    m_displayName = name;

    emit displayNameChanged(name);
}

void ADDomainUser::setUserName(const QString &name)
{
    if (m_userName == name) {
        return;
    }

    m_userName = name;
}

void ADDomainUser::setUserInter(UserInter *user_inter)
{
    if (m_userInter == user_inter) {
        return;
    }

    m_userInter = user_inter;
}

void ADDomainUser::setUid(uid_t uid)
{
    if (m_uid == uid) {
        return;
    }

    m_uid = uid;
}

void ADDomainUser::setIsServerUser(bool is_server)
{
    if (m_isServer == is_server) {
        return;
    }

    m_isServer = is_server;
}

QString ADDomainUser::displayName() const
{
    return m_displayName.isEmpty() ? m_userName : m_displayName;
}

QString ADDomainUser::avatarPath() const
{
    return QString(":/img/default_avatar.svg");
}

QString ADDomainUser::greeterBackgroundPath() const
{
    return QString("/usr/share/wallpapers/deepin/desktop.jpg");
}

QString ADDomainUser::desktopBackgroundPath() const
{
    return QString("/usr/share/wallpapers/deepin/desktop.jpg");
}

bool ADDomainUser::isPasswordExpired() const
{
    if (m_userInter != nullptr) {
        QDBusPendingReply<bool> replay = m_userInter->IsPasswordExpired();
        replay.waitForFinished();
        return !replay.isError() && m_userInter->IsPasswordExpired();
    }
    return false;
}
