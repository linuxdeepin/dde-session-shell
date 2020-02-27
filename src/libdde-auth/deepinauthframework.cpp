#include "deepinauthframework.h"
#include "interface/deepinauthinterface.h"
#include "src/session-widgets/userinfo.h"

#include <QThread>
#include <QTimer>
#include <QVariant>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>

static QString PASSWORD;

DeepinAuthFramework::DeepinAuthFramework(DeepinAuthInterface *inter, QObject *parent)
    : QObject(parent)
    , m_interface(inter)
{
    m_authThread = new QThread;
    m_authagent = new AuthAgent(this);
    m_authagent->moveToThread(m_authThread);
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
    if (user->isLock()) return;

    if (m_authagent == nullptr) {
        if (user->isNoPasswdGrp() || (!user->isNoPasswdGrp() && !PASSWORD.isEmpty())) {
            qDebug() << Q_FUNC_INFO << "keyboard auth start";
            QTimer::singleShot(0, m_authagent, [ = ]() {
                m_authagent->Authenticate(user->name());
            });
        }
    }
}

void DeepinAuthFramework::Clear()
{
    if (!m_authagent.isNull()) {
        delete m_authagent;
        m_authagent = nullptr;
    }

    PASSWORD.clear();
}

void DeepinAuthFramework::inputPassword(const QString &password)
{
    PASSWORD = password;
}

const QString DeepinAuthFramework::RequestEchoOff(const QString &msg)
{
    Q_UNUSED(msg);

    return PASSWORD;
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
