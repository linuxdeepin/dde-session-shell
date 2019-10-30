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
        KEYBOARD = 0x01,
        FPRINT = 0x02,
        ALL = KEYBOARD | FPRINT
    } ;

    explicit DeepinAuthFramework(DeepinAuthInterface *inter, QObject *parent = nullptr);
    ~DeepinAuthFramework();

    friend class AuthAgent;

    bool isAuthenticate() const;

public slots:
    void SetUser(std::shared_ptr<User> user);
    void Authenticate();
    void Clear();
    void setPassword(const QString &password);
    void setAuthType(AuthType type);

private:
    const QString RequestEchoOff(const QString &msg);
    const QString RequestEchoOn(const QString &msg);
    void DisplayErrorMsg(AuthAgent::Type type, const QString &msg);
    void DisplayTextInfo(AuthAgent::Type type, const QString &msg);
    void RespondResult(AuthAgent::Type type, const QString &msg);

private:
    DeepinAuthInterface *m_interface;
    QPointer<AuthAgent> m_keyboard;
    QPointer<AuthAgent> m_fprint;

    AuthType m_type;
};

#endif // DEEPINAUTHFRAMEWORK_H
