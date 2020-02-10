#ifndef DEEPINAUTHINTERFACE_H
#define DEEPINAUTHINTERFACE_H

#include "../authagent.h"
#include <QString>

class DeepinAuthInterface
{
public:
    virtual void onDisplayErrorMsg(AuthAgent::AuthenticationFlag type, const QString &msg) = 0;
    virtual void onDisplayTextInfo(AuthAgent::AuthenticationFlag type, const QString &msg) = 0;
    virtual void onPasswordResult(AuthAgent::AuthenticationFlag type, const QString &msg) = 0;
};

#endif // DEEPINAUTHINTERFACE_H
