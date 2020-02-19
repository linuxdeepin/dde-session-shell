#ifndef AUTHAGENT_H
#define AUTHAGENT_H

#include <security/pam_appl.h>
#include <QObject>

#define MAX_VERIFY_FAILED 5

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

    enum FingerprintStatus {
        MATCH = 0,
        NO_MATCH,
        ERROR,
        RETRY,
        DISCONNECTED
    };

    enum FpRetryStatus {
        SWIPE_TOO_SHORT = 1,
        FINGER_NOT_CENTERED,
        REMOVE_AND_RETRY
    };

    explicit AuthAgent(const QString& user_name, AuthFlag type, QObject *parent = nullptr);
    ~AuthAgent();

    QString UserName() const;
    void Authenticate();
    void Cancel();
    DeepinAuthFramework *parent();

private:
    QString PamService(AuthFlag type) const;
    void pamFingerprintMessage(const QString& message);
    static int funConversation(int num,
                               const struct pam_message** msg,
                               struct pam_response** resp,
                               void* app_data);

private:
    pam_handle_t* m_pamHandle = nullptr;
    bool m_lastStatus = false;
    int  m_verifyFailed = MAX_VERIFY_FAILED;

    AuthFlag m_type = Password;
    QString m_username;
    QString m_password;
};

#endif // AUTHAGENT_H
