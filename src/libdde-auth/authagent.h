#ifndef AUTHAGENT_H
#define AUTHAGENT_H

#include "authority_interface.h"
#include <QObject>

class DeepinAuthFramework;
class AuthAgent : public QObject {
    Q_OBJECT
public:
    enum Type {
        Keyboard,
        Fprint
    };

    explicit AuthAgent(Type type, QObject *parent = nullptr);
    ~AuthAgent();

    void SetUser(const QString &username);
    void Authenticate();
    void Cancel();

public slots:
    const QString RequestEchoOff(const QString &msg);
    const QString RequestEchoOn(const QString &msg);
    void DisplayErrorMsg(const QString &errtype, const QString &msg);
    void DisplayTextInfo(const QString &msg);
    void RespondResult(const QString &msg);

    DeepinAuthFramework *parent();

private:
    AuthorityInterface *m_authority;
    Type m_type;
};

#endif // AUTHAGENT_H
