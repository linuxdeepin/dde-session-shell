#ifndef DEEPINAUTHFRAMEWORK_H
#define DEEPINAUTHFRAMEWORK_H

#include "authagent.h"

#include <QObject>
#include <QPointer>
#include <memory>

class DeepinAuthInterface;
class User;
class DeepinAuthFramework : public QObject
{
    Q_OBJECT
public:
    enum AuthType {
        UnknowAuthType,
        LockType,
        LightdmType
    };

    explicit DeepinAuthFramework(DeepinAuthInterface *inter, QObject *parent = nullptr);
    ~DeepinAuthFramework();

    friend class AuthAgent;

    bool isAuthenticate() const;

public slots:
    void SetUser(std::shared_ptr<User> user);
    void Authenticate();
    void keyBoardAuth();
    void fprintAuth();
    void Clear();
    void setPassword(const QString &password);
    void setAuthType(AuthType type);
    void setCurrentUid(uint uid);

private:
    const QString RequestEchoOff(const QString &msg);
    const QString RequestEchoOn(const QString &msg);
    void DisplayErrorMsg(AuthAgent::AuthFlag type, const QString &msg);
    void DisplayTextInfo(AuthAgent::AuthFlag type, const QString &msg);
    void RespondResult(AuthAgent::AuthFlag type, const QString &msg);

private:
    DeepinAuthInterface *m_interface;
    QPointer<AuthAgent> m_keyboard;
    QPointer<AuthAgent> m_fprint;

    AuthType m_type;
    uint m_currentUserUid;
};

#endif // DEEPINAUTHFRAMEWORK_H
