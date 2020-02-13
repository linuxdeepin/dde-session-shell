#include "authagent.h"
#include "deepinauthframework.h"

#include <QDebug>
#include <stdlib.h>

#ifdef PAM_SUN_CODEBASE
#define PAM_MSG_MEMBER(msg, n, member) ((*(msg))[(n)].member)
#else
#define PAM_MSG_MEMBER(msg, n, member) ((msg)[(n)]->member)
#endif

AuthAgent::AuthAgent(const QString& user_name, AuthFlag type, QObject *parent)
    : QObject(parent)
    , m_type(type)
    , m_username(user_name)
{
    pam_conv conv = { funConversation, static_cast<void*>(this) };
    QString pam_service = PamService(type);
    int ret = pam_start(pam_service.toLocal8Bit().data(), user_name.toLocal8Bit().data(), &conv, &m_pamHandle);
    if( ret != PAM_SUCCESS) {
        qDebug() << Q_FUNC_INFO << pam_strerror(m_pamHandle, ret);
    }
}

AuthAgent::~AuthAgent()
{
    Cancel();
}

QString AuthAgent::UserName() const
{
    return m_username;
}

void AuthAgent::Authenticate()
{
    m_lastStatus = pam_authenticate(m_pamHandle, 0);
    QString msg = QString();

    if(m_lastStatus == PAM_SUCCESS) {
        msg = parent()->RequestEchoOff("");
    } else{
        qDebug() << Q_FUNC_INFO << pam_strerror(m_pamHandle, m_lastStatus);
    }

    parent()->RespondResult(m_type, msg);
}

void AuthAgent::Cancel()
{
    pam_end(m_pamHandle, m_lastStatus);
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
    }
    return "common-auth";
}

int AuthAgent::funConversation(int num_msg, const struct pam_message **msg,
                               struct pam_response **resp, void *app_data)
{
    AuthAgent *app_ptr = static_cast<AuthAgent *>(app_data);
    struct pam_response *aresp = nullptr;
    int idx = 0;

    if(app_ptr == nullptr) {
        qDebug() << "pam: application is null";
        return PAM_CONV_ERR;
    }

    if(num_msg <= 0 || num_msg > PAM_MAX_NUM_MSG)
        return PAM_CONV_ERR;

    if((aresp = static_cast<struct pam_response*>(calloc(num_msg, sizeof(*aresp)))) == nullptr)
        return PAM_BUF_ERR;

    for (idx = 0; idx < num_msg; ++idx) {
        switch(PAM_MSG_MEMBER(msg, idx, msg_style)) {
        case PAM_PROMPT_ECHO_OFF: {
            QString password = app_ptr->parent()->RequestEchoOff(PAM_MSG_MEMBER(msg, idx, msg));

            aresp[idx].resp = strdup(password.toLocal8Bit().data());
            if(aresp[idx].resp == nullptr)
              goto fail;

            aresp[idx].resp_retcode = PAM_SUCCESS;
            break;
        }

        case PAM_PROMPT_ECHO_ON: {
            QString user_name = app_ptr->UserName();
            if((aresp[idx].resp = strdup(user_name.toLocal8Bit().data())) == nullptr)
               goto fail;
            aresp[idx].resp_retcode = PAM_SUCCESS;
            break;
        }

        case  PAM_ERROR_MSG: {
            app_ptr->parent()->DisplayErrorMsg(app_ptr->m_type, PAM_MSG_MEMBER(msg, idx, msg));
            aresp[idx].resp_retcode = PAM_SUCCESS;
            break;
        }

        case  PAM_TEXT_INFO: {
            app_ptr->parent()->DisplayTextInfo(app_ptr->m_type, PAM_MSG_MEMBER(msg, idx, msg));
            aresp[idx].resp_retcode = PAM_SUCCESS;
            break;
         }

        default:
            goto fail;
        }
    }
    *resp = aresp;
    return PAM_SUCCESS;

fail:
    for(idx = 0; idx < num_msg; idx++) {
        free(aresp[idx].resp);
    }
    free(aresp);
    return PAM_CONV_ERR;
}

DeepinAuthFramework *AuthAgent::parent() {
    return qobject_cast<DeepinAuthFramework*>(QObject::parent());
}
