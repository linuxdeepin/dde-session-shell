#ifndef USERINFO_H
#define USERINFO_H

#include <QObject>
#include <com_deepin_daemon_accounts_user.h>
#include <com_deepin_daemon_authenticate.h>
#include <memory>
#include "public_func.h"
#include "constants.h"

#define ACCOUNT_DBUS_SERVICE "com.deepin.daemon.Accounts"
#define ACCOUNT_DBUS_PATH "/com/deepin/daemon/Accounts"
#define DEFAULT_BACKGROUND "/usr/share/backgrounds/default_background.jpg"

using UserInter = com::deepin::daemon::accounts::User;
using com::deepin::daemon::Authenticate;

QString userPwdName(__uid_t uid);

class User : public QObject
{
    Q_OBJECT

public:
    enum UserType
    {
        Native,
        ADDomain,
    };

    struct lockLimit {
        uint lockTime; //当前的锁定时间
        bool isLock = false; //当前锁定状态
        bool isFinished = false;
    } m_lockLimit;

    struct LimitsInfo {
        int unlockSecs;     // 本次锁定总解锁时间（秒），不会随着时间推移减少
        int maxTries;       // 最大重试次数
        int numFailures;    // 失败次数，一直累加
        bool locked;        // 账户锁定状态 --- true: 锁定  false: 解锁
        QString unlockTime; // 解锁时间（本地时间）
    };

    explicit User(QObject *parent);
    User(const User &user);

signals:
    void displayNameChanged(const QString &displayname) const;
    void logindChanged(bool islogind) const;
    void avatarChanged(const QString &avatar) const;
    void greeterBackgroundPathChanged(const QString &path) const;
    void desktopBackgroundPathChanged(const QString &path) const;
    void localeChanged(const QString &locale) const;
    void kbLayoutListChanged(const QStringList &list);
    void currentKBLayoutChanged(const QString &layout);
    void lockChanged(bool lock);
    void lockLimitFinished(bool lock);
    void noPasswdLoginChanged(bool no_passw);
    void use24HourFormatChanged(bool use24);

    void limitsInfoChanged(const QMap<int, LimitsInfo> *);

public:
    bool operator==(const User &user) const;
    const QString name() const { return m_userName; }

    bool isLogin() const { return m_isLogind; }
    uid_t uid() const { return m_uid; }

    const QString locale() const { return m_locale; }
    void setLocale(const QString &locale);

    virtual bool isNoPasswdGrp() const;
    virtual bool isUserIsvalid() const;
    virtual bool isDoMainUser() const { return m_isServer; }
    virtual bool is24HourFormat() const { return true; }

    void setisLogind(bool isLogind);
    virtual void setCurrentLayout(const QString &layout) { Q_UNUSED(layout); }

    void setPath(const QString &path);
    const QString path() const { return m_path; }

    void updateLockLimit(bool is_lock, uint lock_time, uint rest_second = 0);
    uint lockTime() const { return m_lockLimit.lockTime; }
    bool isLock() const { return m_lockLimit.isLock; }

    virtual UserType type() const = 0;
    virtual QString displayName() const { return m_userName; }
    virtual QString avatarPath() const = 0;
    virtual QString greeterBackgroundPath() const = 0;
    virtual QString desktopBackgroundPath() const = 0;
    virtual QStringList kbLayoutList() { return QStringList(); }
    virtual QString currentKBLayout() { return QString(); }
    void onLockTimeOut();

    inline QMap<int, LimitsInfo> *limitsInfo() const { return m_limitsInfo; }
    void updateLimitsInfo(const QString &info);

protected:
    bool m_isLogind;
    bool m_isServer = false;
    bool m_noPasswdGrp = true;

    uid_t m_uid = INT_MAX;
    QString m_userName;
    QString m_fullName;
    QString m_locale;
    QString m_path;
    std::shared_ptr<QTimer> m_lockTimer;
    QMap<int, LimitsInfo> *m_limitsInfo;
};

class NativeUser : public User
{
    Q_OBJECT

public:
    NativeUser(const QString &path, QObject *parent = nullptr);
    UserInter *getUserInter() { return m_userInter; }

    void setCurrentLayout(const QString &currentKBLayout) override;
    UserType type() const override { return Native; }
    QString displayName() const override;
    QString avatarPath() const override;
    QString greeterBackgroundPath() const override;
    QString desktopBackgroundPath() const override;
    QStringList kbLayoutList() override;
    QString currentKBLayout() override;
    bool isNoPasswdGrp() const override;
    bool isUserIsvalid() const override;
    bool is24HourFormat() const override;

private:
    void configAccountInfo(const QString& account_config);
    QStringList readDesktopBackgroundPath(const QString &path);

private:
    UserInter *m_userInter;

    QString m_avatar;
    QString m_greeterBackground;
    QString m_desktopBackground;
    bool m_is24HourFormat;
};

class ADDomainUser : public User
{
    Q_OBJECT

public:
    ADDomainUser(uid_t uid, QObject *parent = nullptr);

    void setUserDisplayName(const QString &name);
    void setUserName(const QString &name);
    void setUserInter(UserInter *user_inter);
    void setUid(uid_t uid);
    void setIsServerUser(bool is_server);

    QString displayName() const override;
    UserType type() const override { return ADDomain; }
    QString avatarPath() const override;
    QString greeterBackgroundPath() const override;
    QString desktopBackgroundPath() const override;

private:
    QString m_displayName;
    UserInter *m_userInter = nullptr;
};

#endif // USERINFO_H
