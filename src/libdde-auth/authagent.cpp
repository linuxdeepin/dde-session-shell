#include "authagent.h"
#include "deepinauthframework.h"

#include <QThread>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <stdlib.h>

#ifdef PAM_SUN_CODEBASE
#define PAM_MSG_MEMBER(msg, n, member) ((*(msg))[(n)].member)
#else
#define PAM_MSG_MEMBER(msg, n, member) ((msg)[(n)]->member)
#endif

#define PAM_SERVICE_NAME "common-auth"

AuthAgent::AuthAgent(DeepinAuthFramework *deepin)
    : m_deepinauth(deepin)
    , m_mutex(new QMutex)
{
    connect(this, &AuthAgent::displayErrorMsg, deepin, &DeepinAuthFramework::DisplayErrorMsg, Qt::QueuedConnection);
    connect(this, &AuthAgent::displayTextInfo, deepin, &DeepinAuthFramework::DisplayErrorMsg, Qt::QueuedConnection);
    connect(this, &AuthAgent::respondResult, deepin, &DeepinAuthFramework::RespondResult, Qt::QueuedConnection);
}

AuthAgent::~AuthAgent()
{
    if(m_mutex != nullptr) {
        delete m_mutex;
        m_mutex = nullptr;
    }

    Cancel();
}

void AuthAgent::Responsed(const QString &password)
{
    m_password = password;
    if(!m_mutex->tryLock())
        m_mutex->unlock();
}

void AuthAgent::Authenticate(const QString& username)
{
    pam_conv conv = { funConversation, static_cast<void*>(this) };
    int ret = pam_start(PAM_SERVICE_NAME, username.toLocal8Bit().data(), &conv, &m_pamHandle);
    if( ret != PAM_SUCCESS) {
        qDebug() << Q_FUNC_INFO << pam_strerror(m_pamHandle, ret);
    }

    m_lastStatus = pam_authenticate(m_pamHandle, 0);
    QString msg = QString();

    if(m_lastStatus == PAM_SUCCESS) {
        msg = deepinAuth()->RequestEchoOff("");
    } else{
        qDebug() << Q_FUNC_INFO << pam_strerror(m_pamHandle, m_lastStatus);
    }

    emit respondResult(msg);
}

void AuthAgent::Cancel()
{
    pam_end(m_pamHandle, m_lastStatus);
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
            if(app_ptr->m_mutex->tryLock()) app_ptr->m_mutex->lock();

            QString password = app_ptr->deepinAuth()->RequestEchoOff(PAM_MSG_MEMBER(msg, idx, msg));
            aresp[idx].resp = strdup(password.toLocal8Bit().data());

            if(aresp[idx].resp == nullptr)
              goto fail;

            aresp[idx].resp_retcode = PAM_SUCCESS;
            break;
        }

        case PAM_PROMPT_ECHO_ON: {
            app_ptr->displayTextInfo(QString::fromLocal8Bit(PAM_MSG_MEMBER(msg, idx, msg)));
            aresp[idx].resp_retcode = PAM_SUCCESS;
            break;
        }

        case  PAM_ERROR_MSG:
        case  PAM_TEXT_INFO: {
            qDebug() << "pam authagent: " << PAM_MSG_MEMBER(msg, idx, msg);
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
