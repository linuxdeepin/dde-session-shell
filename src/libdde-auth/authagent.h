#ifndef AUTHAGENT_H
#define AUTHAGENT_H

#include <security/pam_appl.h>
#include <security/pam_misc.h>
#include <QObject>

class DeepinAuthFramework;
class AuthAgent : public QObject {
    Q_OBJECT
public:
    enum AuthFlag {
        Password = 1 << 0,
        Fingerprint = 1 << 1,
        Face = 1 << 2,
        ActiveDirectory = 1 << 3
    };

    enum StatusCode {
        SUCCESS = 0,
        FAILURE,
        CANCEL,
    };

    explicit AuthAgent(const QString& user_name, AuthFlag type, QObject *parent = nullptr);
    ~AuthAgent();

    void SetPassword(const QString &password);
    void Authenticate();
    void Cancel();
    DeepinAuthFramework *parent();

private:
    QString PamService(AuthFlag type) const;
    static int funConversation(int num,
                               const struct pam_message** msg,
                               struct pam_response** resp,
                               void* app_data);

private:
    pam_handle_t* m_pamHandle = nullptr;
    pam_response* m_pamResponse = nullptr;
    pam_conv m_pamConversation;
    bool m_lastStatus = false;

    AuthFlag m_type = Password;
    QString m_password;
};

#endif // AUTHAGENT_H
