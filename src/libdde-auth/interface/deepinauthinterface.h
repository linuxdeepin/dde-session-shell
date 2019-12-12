#ifndef DEEPINAUTHINTERFACE_H
#define DEEPINAUTHINTERFACE_H

#include "../authagent.h"
#include <QString>

class DeepinAuthInterface
{
public:
    virtual void onDisplayErrorMsg(AuthAgent::Type type, const QString &error_type, const QString &msg) = 0;
    virtual void onDisplayTextInfo(AuthAgent::Type type, const QString &msg) = 0;
    virtual void onPasswordResult(AuthAgent::Type type, const QString &msg) = 0;
};

#endif // DEEPINAUTHINTERFACE_H
