#ifndef USERINFO_H
#define USERINFO_H

#include <QObject>
#include <com_deepin_daemon_accounts_user.h>
#include <memory>
#include "../global_util/public_func.h"
#include "../global_util/constants.h"

#define ACCOUNT_DBUS_SERVICE "com.deepin.daemon.Accounts"
#define ACCOUNT_DBUS_PATH "/com/deepin/daemon/Accounts"

using UserInter = com::deepin::daemon::accounts::User;

class User : public QObject
{
    Q_OBJECT

public:
    enum UserType
    {
        Native,
        ADDomain,
    };

    struct lockData {
        std::vector<uint> waitTime; //可设置的等待时间限制数组
        uint limitTryNum; //可设置的锁定次数
        uint lockTime; //当前的锁定时间
        uint tryNum; //尝试的次数记录
    } m_lockData;

    User(QObject *parent);
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
    void noPasswdLoginChanged(bool no_passw);
    void use24HourFormatChanged(bool use24);

public:
    bool operator==(const User &user) const;
    const QString name() const { return m_userName; }

    bool isLogin() const { return m_isLogind; }
    uid_t uid() const { return m_uid; }

    const QString locale() const { return m_locale; }
    void setLocale(const QString &locale);

    virtual bool isNoPasswdGrp() const;
    virtual bool isPasswordExpired() const { return false; }
    virtual bool isUserIsvalid() const;
    virtual bool isDoMainUser() const { return m_isServer; }
    virtual bool is24HourFormat() const { return true; }

    void setisLogind(bool isLogind);
    virtual void setCurrentLayout(const QString &layout) { Q_UNUSED(layout); }

    void setPath(const QString &path);
    const QString path() const { return m_path; }

    uint lockTime() const { return m_lockData.lockTime; }
    bool isLock() const { return m_isLock; }
    bool isLockForNum();
    void startLock();
    void resetLock();

    virtual UserType type() const = 0;
    virtual QString displayName() const { return m_userName; }
    virtual QString avatarPath() const = 0;
    virtual QString greeterBackgroundPath() const = 0;
    virtual QString desktopBackgroundPath() const = 0;
    virtual QStringList kbLayoutList() { return QStringList(); }
    virtual QString currentKBLayout() { return QString(); }
    void onLockTimeOut();

protected:
    bool m_isLogind;
    bool m_isLock;
    bool m_isServer = false;
    bool m_noPasswdGrp = true;
    bool m_isPasswdExpired = false;

    uid_t m_uid = INT_MAX;
    QString m_userName;
    QString m_fullName;
    QString m_locale;
    QString m_path;
    std::shared_ptr<QTimer> m_lockTimer;
    time_t m_startTime;//记录账号锁定的时间搓
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
    bool isPasswordExpired() const override;
    bool isUserIsvalid() const override;
    bool is24HourFormat() const override;

private:
    UserInter *m_userInter;
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
    bool isPasswordExpired() const override;

private:
    QString m_displayName;
    UserInter *m_userInter = nullptr;
};

#endif // USERINFO_H
