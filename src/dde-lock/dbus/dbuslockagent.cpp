#include "dbuslockagent.h"
#include "src/session-widgets/sessionbasemodel.h"
#include "src/global_util/public_func.h"

#include <QGSettings>

DBusLockAgent::DBusLockAgent(QObject *parent) : QObject(parent)
{

}

void DBusLockAgent::setModel(SessionBaseModel *const model)
{
    m_model = model;
}

void DBusLockAgent::Show()
{
    if (!isDeepinAuth()) {
        return;
    }
    InternalShow();
}

void DBusLockAgent::ShowAuth(bool active)
{
    if (!isDeepinAuth()) {
        return;
    }
    Show();
    emit m_model->activeAuthChanged(!active);
}

// 待机，enable=true：进入待机；enable=false：待机恢复
void DBusLockAgent::Suspend(bool enable)
{
    if (enable) {
        m_model->setIsBlackModel(true);
        m_model->setIsShow(true);
    } else {
        QDBusInterface infc("com.deepin.daemon.Power","/com/deepin/daemon/Power","com.deepin.daemon.Power");
        // 待机恢复需要密码
        bool bSuspendLock = infc.property("SleepLock").toBool();

        if (bSuspendLock) {
            m_model->setIsBlackModel(false);
            m_model->setIsShow(true);
        } else {
            m_model->setIsShow(false);
            emit m_model->visibleChanged(false);
        }
    }
}

void DBusLockAgent::Hibernate(bool enable)
{
    m_model->setIsHibernateModel(enable);
    m_model->setIsShow(true);
}

/**
     * @brief 自动切换到此用户的TTY并显示锁屏程序。此功能虽然为域管开发，但非域管情景也可以使用
     * 
*/
void DBusLockAgent::SwitchTTYAndShow() 
{
    SwitchTTY();
    InternalShow();
}

void DBusLockAgent::SwitchTTY() 
{
    QDBusMessage send = QDBusMessage::createMethodCall("org.freedesktop.login1","/org/freedesktop/login1/session/self","org.freedesktop.login1.Session","Activate");
    QDBusMessage ret = QDBusConnection::systemBus().call(send);
    if (ret.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "call org.freedesktop.login1 /org/freedesktop/login1/session/self org.freedesktop.login1.Session Activate error: " << ret;
    }
#ifdef QT_DEBUG
    qDebug() << ret;
#endif
}

void DBusLockAgent::InternalShow() 
{
    m_model->setIsBlackModel(false);
    m_model->setIsHibernateModel(false);
    m_model->setIsShow(true);
}

void DBusLockAgent::ShowUserList()
{
    if (!isDeepinAuth()) {
        return;
    }
    emit m_model->showUserList();
}


