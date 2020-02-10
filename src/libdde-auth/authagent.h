#ifndef AUTHAGENT_H
#define AUTHAGENT_H

#include "authenticate_interface.h"
#include <QObject>

class DeepinAuthFramework;
class AuthAgent : public QObject {
    Q_OBJECT
public:
    enum AuthenticationFlag {
        Password = 1 << 0,
        Fingerprint = 1 << 1,
        Face = 1 << 2,
        ActiveDirectory = 1 << 3
    };

    enum Status {
        SuccessCode = 0,
        FailureCode,
        CancelCode,
    };

    explicit AuthAgent(const QString& name, AuthenticationFlag type, QObject *parent = nullptr);
    ~AuthAgent();

    void SetPassword(const QString &password);
    void Authenticate();
    void Cancel();

public slots:
    void onStatus(const QString &id, int code, const QString &msg);
    DeepinAuthFramework *parent();

private:
    AuthenticateInterface *m_authenticate;
    AuthenticationFlag m_type = Password;
    QString m_name;
    QString m_authId;
};

#endif // AUTHAGENT_H
