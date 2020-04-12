#include "deepinauthframework.h"
#include "interface/deepinauthinterface.h"
#include "src/session-widgets/userinfo.h"

#include <QThread>
#include <QTimer>
#include <QVariant>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>

DeepinAuthFramework::DeepinAuthFramework(DeepinAuthInterface *inter, QObject *parent)
    : QObject(parent)
    , m_interface(inter)
{
    m_authThread = new QThread;
    m_authThread->start();
}

DeepinAuthFramework::~DeepinAuthFramework()
{
    if(m_authThread != nullptr) {
        if(m_authThread->isRunning()) m_authThread->quit();
        m_authThread->deleteLater();
    }
}

bool DeepinAuthFramework::isAuthenticate() const
{
    return !m_authagent.isNull();
}

void DeepinAuthFramework::Authenticate(std::shared_ptr<User> user)
{
    m_currentUser = user;
    if (user->isLock()) return;

    qDebug() << Q_FUNC_INFO << "pam auth start";
    m_authagent = new AuthAgent(this);
    m_authagent->moveToThread(m_authThread);

    QTimer::singleShot(100, m_authagent, [ = ]() {
        m_authagent->Authenticate(user->name());
    });
}

void DeepinAuthFramework::Clear()
{
    if (!m_authagent.isNull()) {
        delete m_authagent;
        m_authagent = nullptr;
    }

    m_password.clear();
}

void DeepinAuthFramework::Responsed(const QString &password)
{
    if(m_authagent.isNull() || !m_authThread->isRunning()) {
        qDebug() << "pam auth agent is not start";
        return;
    }

    m_password = password;
    if (m_currentUser->isNoPasswdGrp() || (!m_currentUser->isNoPasswdGrp() && !m_password.isEmpty())) {
        qDebug() << "pam responsed password";

        m_authagent->Responsed(password);
    }
}

const QString DeepinAuthFramework::RequestEchoOff(const QString &msg)
{
    Q_UNUSED(msg);

    return m_password;
}

const QString DeepinAuthFramework::RequestEchoOn(const QString &msg)
{
    return msg;
}

void DeepinAuthFramework::DisplayErrorMsg(const QString &msg)
{
    m_interface->onDisplayErrorMsg(msg);
}

void DeepinAuthFramework::DisplayTextInfo(const QString &msg)
{
    m_interface->onDisplayTextInfo(msg);
}

void DeepinAuthFramework::RespondResult(const QString &msg)
{
    m_interface->onPasswordResult(msg);
}
