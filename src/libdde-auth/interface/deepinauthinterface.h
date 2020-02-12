#ifndef DEEPINAUTHINTERFACE_H
#define DEEPINAUTHINTERFACE_H

#include "../authagent.h"
#include <QString>

class DeepinAuthInterface
{
public:
    virtual void onDisplayErrorMsg(AuthAgent::AuthFlag type, const QString &msg) = 0;
    virtual void onDisplayTextInfo(AuthAgent::AuthFlag type, const QString &msg) = 0;
    virtual void onPasswordResult(AuthAgent::AuthFlag type, const QString &msg) = 0;
};

#endif // DEEPINAUTHINTERFACE_H
