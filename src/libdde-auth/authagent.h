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

    explicit AuthAgent(DeepinAuthFramework *deepin);
    ~AuthAgent();

    void SetPassword(const QString& password);
    void Authenticate(const QString& username);
    void Cancel();
    DeepinAuthFramework *deepinAuth() { return m_deepinauth; }

signals:
    void displayErrorMsg(const QString &msg);
    void displayTextInfo(const QString &msg);
    void respondResult(const QString &msg);

private:
    void pamFingerprintMessage(const QString& message);
    static int funConversation(int num,
                               const struct pam_message** msg,
                               struct pam_response** resp,
                               void* app_data);

private:
    DeepinAuthFramework* m_deepinauth = nullptr;
    pam_handle_t* m_pamHandle = nullptr;
    int  m_lastStatus = 255;
    int  m_verifyFailed = MAX_VERIFY_FAILED;

    QString m_password;
};

#endif // AUTHAGENT_H
