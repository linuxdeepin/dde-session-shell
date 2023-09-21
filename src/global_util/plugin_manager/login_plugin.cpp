// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "login_plugin.h"
#include "public_func.h"

LoginPlugin::LoginPlugin(dss::module::BaseModuleInterface *module, QObject *parent)
    : PluginBase(module, parent)
    , m_authType(AuthType::AT_Password)
{

}

bool LoginPlugin::isPluginEnabled()
{
    QJsonObject message;
    message["CmdType"] = "IsPluginEnabled";
    const QString &result = this->message(toJson(message));
    qDebug() << "Is plugin enabled: " << result;
    const QJsonObject &dataObj = getDataObj(result);
    if (dataObj.isEmpty() || !dataObj.contains("IsPluginEnabled"))
        return true;

    return dataObj["IsPluginEnabled"].toBool(true);
}

int LoginPlugin::level()
{
    QJsonObject message {
        {"CmdType", "GetLevel"}
    };

    const QString &result = this->message(toJson(message));
    const QJsonObject &dataObj = getDataObj(result);
    if (!dataObj.contains("Level")) {
        return 1;
    }

    return dataObj["Level"].toInt(1);
}

int LoginPlugin::loginType()
{
    QJsonObject message {
        {"CmdType", "GetLoginType"}
    };

    const QString &result = this->message(toJson(message));
    qDebug() << "Login type, result: " << result;
    const QJsonObject &dataObj = getDataObj(result);
    if (!dataObj.contains("LoginType")) {
        return CustomLoginType::CLT_Default;
    }

    return dataObj["LoginType"].toInt(CustomLoginType::CLT_Default);
}

bool LoginPlugin::hasSecondLevel(const QString &user)
{
    QJsonObject message {
        {"CmdType", "HasSecondLevel"},
        {"Data", user}
    };

    const QString &result = this->message(toJson(message));
    const QJsonObject &dataObj = getDataObj(result);
    if (!dataObj.contains("HasSecondLevel")) {
        return false;
    }

    return dataObj["HasSecondLevel"].toBool(false);
}

int LoginPlugin::updateLoginType()
{
    QJsonObject message {
        {"CmdType", "UpdateLoginType"}
    };

    const QString &result = this->message(toJson(message));
    qDebug() << "Update login type, result: " << result;
    const QJsonObject &dataObj = getDataObj(result);
    if (!dataObj.contains("LoginType")) {
        return CustomLoginType::CLT_Default;
    }

    return dataObj["LoginType"].toInt(CustomLoginType::CLT_Default);
}

int LoginPlugin::sessionTimeout()
{
    QJsonObject message {
        {"CmdType", "GetSessionTimeout"}
    };

    const QString &result = this->message(toJson(message));
    qDebug() << "Get session timeout, result: " << result;
    const QJsonObject &dataObj = getDataObj(result);
    if (!dataObj.contains("SessionTimeout")) {
        return DEFAULT_SESSION_TIMEOUT;
    }

    return dataObj["SessionTimeout"].toInt(DEFAULT_SESSION_TIMEOUT);
}

bool LoginPlugin::supportDefaultUser()
{
    QJsonObject message;
    message["CmdType"] = "GetConfigs";
    const QString &result = this->message(toJson(message));
    const QJsonObject &dataObj = getDataObj(result);
    if (dataObj.isEmpty())
        return true;

    return dataObj["SupportDefaultUser"].toBool(true);
}

void LoginPlugin::notifyCurrentUserChanged(const QString &userName)
{
    QJsonObject message;
    message["CmdType"] = "CurrentUserChanged";
    QJsonObject user;
    user["Name"] = userName;
    message["Data"] = user;

    this->message(toJson(message));
}

void LoginPlugin::updateConfig()
{
    QJsonObject message;
    message["CmdType"] = "GetConfigs";
    QString result = this->message(toJson(message));
    qInfo() << "Get configs result: " << result;

    QJsonParseError jsonParseError;
    const QJsonDocument resultDoc = QJsonDocument::fromJson(result.toLatin1(), &jsonParseError);
    if (jsonParseError.error != QJsonParseError::NoError || resultDoc.isEmpty()) {
        qWarning() << "Result json parse error, error: " << jsonParseError.error;
        return;
    }

    QJsonObject resultObj = resultDoc.object();
    if (!resultObj.contains("Data")) {
        qWarning() << "Result doesn't contains the 'data' field";
        return;
    }

    QJsonObject dataObj = resultObj["Data"].toObject();
    m_pluginConfig.showAvatar = dataObj["ShowAvatar"].toBool(m_pluginConfig.showAvatar);
    m_pluginConfig.showUserName = dataObj["ShowUserName"].toBool(m_pluginConfig.showUserName);
    m_pluginConfig.showSwitchButton = dataObj["ShowSwitchButton"].toBool(m_pluginConfig.showSwitchButton);
    m_pluginConfig.showLockButton = dataObj["ShowLockButton"].toBool(m_pluginConfig.showLockButton);
    m_pluginConfig.defaultAuthLevel = (DefaultAuthLevel)dataObj["DefaultAuthLevel"].toInt();
    m_authType = (AuthType)dataObj["AuthType"].toInt();
}
