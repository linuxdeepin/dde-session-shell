#include "authagent.h"
#include "deepinauthframework.h"

#include <QDebug>

AuthAgent::AuthAgent(const QString& user_name, AuthFlag type, QObject *parent)
    : QObject(parent)
    , m_type(type)
{
    m_pamConversation = { funConversation, static_cast<void*>(this) };
    QString pam_service = PamService(type);
    if(pam_start(pam_service.toLocal8Bit().data(),
                 user_name.toLocal8Bit().data(),
                 &m_pamConversation,
                 &m_pamHandle) != PAM_SUCCESS) {
      qDebug() << Q_FUNC_INFO << "pam start failed";
    }
}

AuthAgent::~AuthAgent()
{
    Cancel();
}

void AuthAgent::SetPassword(const QString &password)
{
    m_password = password;
}

void AuthAgent::Authenticate()
{
    m_pamResponse = new pam_response;
    m_pamResponse->resp = strdup(m_password.toLocal8Bit().data());
    m_pamResponse->resp_retcode = 0;

    m_lastStatus = pam_authenticate(m_pamHandle, 0);
}

void AuthAgent::Cancel()
{
    pam_end(m_pamHandle, m_lastStatus);
    if(m_pamResponse) {
        delete m_pamResponse;
        m_pamResponse = nullptr;
    }
}

QString AuthAgent::PamService(AuthAgent::AuthFlag type) const
{
    switch (type) {
    case AuthFlag::Password:
        return "common-auth";
 
    case AuthFlag::Fingerprint:
        return "fprint-auth";

    case AuthFlag::Face:
        return "face-auth";

    case AuthFlag::ActiveDirectory:
        return "ad-auth";

    default:
        return "common-auth";
   }
}

int AuthAgent::funConversation(int num_msg, const struct pam_message **msg,
                               struct pam_response **resp, void *app_data)
{
    int idx = 0;
    const pam_message *pm;
    AuthAgent* app_ptr = static_cast<AuthAgent*>(app_data);

    if (num_msg > 1) {
        qDebug() << "pam conversation msg num more then 1, maybe error!" << endl;
    }

    for (idx = 0; idx < num_msg; ++idx) {
        pm = *msg + idx;
        switch(pm->msg_style) {
        case PAM_PROMPT_ECHO_OFF: {
            app_ptr->parent()->RequestEchoOff(pm->msg);
            return PAM_SUCCESS;
        }

        case PAM_PROMPT_ECHO_ON: {
            app_ptr->parent()->RequestEchoOn(pm->msg);
            return PAM_SUCCESS;
        }

        case  PAM_ERROR_MSG:
            app_ptr->parent()->DisplayErrorMsg(app_ptr->m_type, pm->msg);
            break;

        case  PAM_TEXT_INFO:
            app_ptr->parent()->DisplayTextInfo(app_ptr->m_type, pm->msg);
            break;
        }
    }

    return PAM_CONV_ERR;
}

DeepinAuthFramework *AuthAgent::parent() {
    return qobject_cast<DeepinAuthFramework*>(QObject::parent());
}
