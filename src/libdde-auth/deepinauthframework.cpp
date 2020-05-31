#include "deepinauthframework.h"
#include "interface/deepinauthinterface.h"
#include "src/session-widgets/userinfo.h"

#include <QThread>
#include <QTimer>
#include <QVariant>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <signal.h>

DeepinAuthFramework::DeepinAuthFramework(DeepinAuthInterface *inter, QObject *parent)
    : QObject(parent)
    , m_interface(inter)
{
    m_authagent = new AuthAgent(this);
}

DeepinAuthFramework::~DeepinAuthFramework()
{
    if (!m_authagent.isNull()) {
        delete m_authagent;
        m_authagent = nullptr;

        if (m_authPId != 0) {
            pthread_kill(m_authPId, SIGQUIT);
            pthread_join(m_authPId, nullptr);
            m_authPId = 0;
        }
    }
}

bool DeepinAuthFramework::isAuthenticate() const
{
    return m_authPId != 0;
}

int DeepinAuthFramework::GetAuthType()
{
    return m_authagent->GetAuthType();
}

void DeepinAuthFramework::Authenticate(std::shared_ptr<User> user)
{
    if (user->isLock()) return;

    m_password.clear();

    m_authagent->setUserName(user->name());

    if (m_authPId != 0) {
        pthread_kill(m_authPId, SIGQUIT);
        pthread_join(m_authPId, nullptr);
        m_authPId = 0;
    }

    qDebug() << Q_FUNC_INFO << "pam auth start" << m_authagent->thread()->loopLevel();

    m_currentUser = user;

    if (m_authPId != 0) {
        qDebug() << "stop authAgent thread error !";
    }

    QTimer::singleShot(100, m_authagent, [ = ]() {
        auto ret = pthread_create(&m_authPId, nullptr, (void* (*)(void *))(void *)(AuthAgent::Authenticate), (void *)m_authagent);
        if (ret < 0) {

        }
        pthread_detach(m_authPId);
    });
}

void DeepinAuthFramework::Responsed(const QString &password)
{
    if(m_authagent.isNull() || m_authPId == 0) {
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
