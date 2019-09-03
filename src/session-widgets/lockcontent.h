#ifndef LOCKCONTENT_H
#define LOCKCONTENT_H

#include <QWidget>
#include <memory>

#include <com_deepin_daemon_imageblur.h>

#include "sessionbasewindow.h"
#include "sessionbasemodel.h"
#include "src/widgets/mediawidget.h"

using ImageBlur = com::deepin::daemon::ImageBlur;

class ControlWidget;
class UserInputWidget;
class User;
class UserFrame;
class ShutdownWidget;
class LogoWidget;
class TimeWidget;
class UserLoginInfo;

class LockContent : public SessionBaseWindow
{
    Q_OBJECT
public:
    explicit LockContent(SessionBaseModel * const model, QWidget *parent = nullptr);

    virtual void onCurrentUserChanged(std::shared_ptr<User> user);
    virtual void onStatusChanged(SessionBaseModel::ModeStatus status);
    virtual void releaseAllKeyboard();
    virtual void restoreCenterContent();
    virtual void restoreMode();

signals:
    void requestBackground(const QString &path);
    void requestAuthUser(const QString &password);
    void requestSwitchToUser(std::shared_ptr<User> user);
    void requestSetLayout(std::shared_ptr<User> user, const QString &value);

public slots:
    void pushUserFrame();
    void pushShutdownFrame();
    void setMPRISEnable(const bool state);

protected:
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;
    void hideEvent(QHideEvent *event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

protected:
    void updateBackground(const QString &path);
    void onBlurDone(const QString &source, const QString &blur, bool status);
    void toggleVirtualKB();
    void updateVirtualKBPosition();
    void onUserListChanged(QList<std::shared_ptr<User>> list);
    bool getUse24HourFormat() const;

protected:
    SessionBaseModel *m_model;
    ControlWidget *m_controlWidget;
    UserInputWidget *m_userInputWidget;
    UserFrame *m_userFrame;
    ShutdownWidget *m_shutdownFrame;
    ImageBlur *m_imageBlurInter;
    QWidget *m_virtualKB;
    std::shared_ptr<User> m_user;
    QList<QMetaObject::Connection> m_currentUserConnects;
    QTranslator *m_translator;
    LogoWidget *m_logoWidget;
    TimeWidget *m_timeWidget;
	MediaWidget *m_mediaWidget;
    UserLoginInfo *m_userLoginInfo;
    QDBusInterface *m_lock24HourFormatInter;
    QDBusInterface *m_greeter24HourFormatInter;
};

#endif // LOCKCONTENT_H
