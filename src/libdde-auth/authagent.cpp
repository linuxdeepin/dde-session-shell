#include "authagent.h"
#include "deepinauthframework.h"

#include <QDBusConnection>
#include <QDateTime>
#include <QDebug>
#include <QDBusObjectPath>

AuthAgent::AuthAgent(const QString& name, AuthenticationFlag type, QObject *parent)
    : QObject(parent)
    , m_type(type)
    , m_name(name)
{
    m_authenticate = new AuthenticateInterface("com.deepin.daemon.Authenticate",
                                "/com/deepin/daemon/Authenticate",
                                QDBusConnection::systemBus(), this);

    connect(m_authenticate, &AuthenticateInterface::Status, this, &AuthAgent::onStatus);
}

AuthAgent::~AuthAgent()
{
    Cancel();
}

void AuthAgent::SetPassword(const QString &password)
{
    Q_ASSERT(!m_authId.isEmpty());
    m_authenticate->SetPassword(m_authId, password);
}

void AuthAgent::Authenticate()
{
    qDebug() << Q_FUNC_INFO << " " << m_name << " " << m_type;
    m_authId = m_authenticate->Authenticate(m_name, m_type, 0);
}

void AuthAgent::Cancel()
{
    Q_ASSERT(!m_authId.isEmpty());
    m_authenticate->CancelAuthenticate(m_authId);
}

void AuthAgent::onStatus(const QString &id, int code, const QString &msg)
{
    Q_ASSERT(!m_authId.isEmpty());
    if(m_authId == id) {
        switch(code) {
        case Status::SuccessCode:
            parent()->RespondResult(m_type, msg);
            break;
        case Status::FailureCode:
            parent()->DisplayErrorMsg(m_type, msg);
            break;
        case Status::CancelCode:
            parent()->DisplayTextInfo(m_type, msg);
            break;
        default:
            break;
        }
    }
}

DeepinAuthFramework *AuthAgent::parent() {
    return qobject_cast<DeepinAuthFramework*>(QObject::parent());
}
