/*
 * Copyright (C) 2021 ~ 2021 Uniontech Software Technology Co.,Ltd.
 *
 * Author:     Fang Shichao <fangshichao@uniontech.com>
 *
 * Maintainer: Fang Shichao <fangshichao@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "login_module.h"

#include <QBoxLayout>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QTimer>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QDBusReply>

#include <DConfig>

DCORE_USE_NAMESPACE

namespace dss {
namespace module {

LoginModule::LoginModule(QObject *parent)
    : QObject(parent)
    , m_callback(nullptr)
    , m_callbackFun(nullptr)
    , m_messageCallbackFunc(nullptr)
    , m_loginWidget(nullptr)
    , m_appType(AppType::Login)
    , m_loadPluginType(Notload)
    , m_isAcceptSignal(false)
    , m_dconfig(DConfig::create(getDefaultConfigFileName(), getDefaultConfigFileName(), QString(), this))
    , m_spinner(nullptr)
{
    setObjectName(QStringLiteral("LoginModule"));

    //华为机型,从override配置中获取是否加载插件
    if (m_dconfig) {
        bool showPlugin = m_dconfig->value("enableOneKeylogin", false).toBool();
        m_loadPluginType = showPlugin ? Load : Notload;
        if (!showPlugin)
            return;
    }


    initConnect();

    QDBusMessage m = QDBusMessage::createMethodCall("com.deepin.daemon.Authenticate", "/com/deepin/daemon/Authenticate/Fingerprint",
                                                                             "com.deepin.daemon.Authenticate.Fingerprint",
                                                                             "IdentifyWithMultipleUser");
    // 将消息发送到Dbus
    QDBusPendingCall identifyCall = QDBusConnection::systemBus().asyncCall(m);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(identifyCall, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, [this, identifyCall, watcher] {
        qDebug() << Q_FUNC_INFO << "Get license state:" << identifyCall.error().message();
        if (!identifyCall.isError()) {
            QDBusMessage response = identifyCall.reply();
            //判断Method是否被正确返回
            if (response.type()== QDBusMessage::ReplyMessage) {
                qDebug() << Q_FUNC_INFO << "dbus IdentifyWithMultipleUser call success";
            } else {
                qWarning() << Q_FUNC_INFO << "dbus IdentifyWithMultipleUser call failed";
                m_isAcceptSignal = false;
                //FIXME 此处不能调用回调，因为还没初始化，此处的逻辑应该在setCallBack函数完成后再进行。
                sendAuthTypeToSession();
            }
        }
        watcher->deleteLater();
    });


    //在2.5秒内没接收到一键登录信号，视为失败
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this] {
        qInfo() << Q_FUNC_INFO << "start 2.5s, m_isAcceptSignal" << m_isAcceptSignal;
        if (!m_isAcceptSignal) {
            sendAuthTypeToSession();
        }
    }, Qt::DirectConnection);
    timer->setInterval(2500);
    timer->setSingleShot(true);
    timer->start();
}

LoginModule::~LoginModule()
{
    if (m_loginWidget) {
        delete m_loginWidget;
    }

    if (m_dconfig) {
        delete m_dconfig;
    }

    if (m_spinner) {
        m_spinner->stop();
        delete m_spinner;
    }
}

void LoginModule::initConnect()
{
    bool isConIsSleep = QDBusConnection::systemBus().connect("org.freedesktop.login1", "/org/freedesktop/login1","org.freedesktop.login1.Manager",
                                                    "PrepareForSleep", this, SLOT(slotPrepareForSleep(bool)));
    qInfo() << Q_FUNC_INFO << "connect SlotPrepareForSleep: " << isConIsSleep;


    bool isConnectSuccess = QDBusConnection::systemBus().connect("com.deepin.daemon.Authenticate", "/com/deepin/daemon/Authenticate/Fingerprint","com.deepin.daemon.Authenticate.Fingerprint",
                                                    "VerifyStatus", this, SLOT(slotIdentifyStatus(const QString &, const int, const QString &)));
    qInfo() << Q_FUNC_INFO << "isconnectsuccess: " << isConnectSuccess;

}

void LoginModule::init()
{
    initUI();
    updateInfo();
}

void LoginModule::initUI()
{
    qInfo() << Q_FUNC_INFO;
    if (m_loginWidget) {
        return;
    }
    m_loginWidget = new QWidget;
    m_loginWidget->setAccessibleName(QStringLiteral("LoginWidget"));
    m_loginWidget->setMinimumSize(260, 100);

    m_loginWidget->setLayout(new QHBoxLayout);
    m_spinner = new DTK_WIDGET_NAMESPACE::DSpinner(m_loginWidget);
    m_spinner->setFixedSize(40, 40);
    m_loginWidget->layout()->addWidget(m_spinner);
    m_spinner->start();

}

void LoginModule::updateInfo()
{
    qInfo() << Q_FUNC_INFO;
    if (!m_messageCallbackFunc) {
        qWarning() << Q_FUNC_INFO << "message callback func is nullptr";
        return;
    }

    // 发送获取属性的请求
    QJsonObject message;
    message.insert("CmdType", "GetProperties");
    QJsonArray array;
    array.append("AppType");
    array.append("CurrentUser");
    message["Data"] = array;
    QJsonDocument doc;
    doc.setObject(message);
    std::string ret = m_messageCallbackFunc(doc.toJson().toStdString(), m_callback->app_data);

    // 解析返回值
    QJsonParseError jsonParseError;
    const QJsonDocument retDoc = QJsonDocument::fromJson(QByteArray::fromStdString(ret), &jsonParseError);
    if (jsonParseError.error != QJsonParseError::NoError || retDoc.isEmpty()) {
        qWarning() << Q_FUNC_INFO << "Failed to analysis AppType info from shell!: " << QString::fromStdString(ret);
        return ;
    }

    QJsonObject obj = retDoc.object();
    if (obj.value("Code").toInt() != 0) {
        qWarning() << "Get properties failed, message: " << obj.value("Message").toString();
        return;
    }

    QJsonObject data = obj.value("Data").toObject();
    if (data.contains("AppType")) {
        m_appType = (AppType)data.value("AppType").toInt();
        qInfo() << Q_FUNC_INFO << "App type: " << m_appType;
    }

    if (data.contains("CurrentUser")) {
        QJsonObject user = data.value("CurrentUser").toObject();
        m_userName = user.value("Name").toString();
        qInfo() << Q_FUNC_INFO << "Current user: " << user;
    }
}

void LoginModule::setCallback(LoginCallBack *callback)
{
    qInfo() << Q_FUNC_INFO;
    m_callback = callback;
    m_callbackFun = callback->authCallbackFun;
    m_messageCallbackFunc = callback->messageCallbackFunc;
}

std::string LoginModule::onMessage(const std::string &message)
{
    qInfo() << Q_FUNC_INFO;
    QJsonParseError jsonParseError;
    const QJsonDocument messageDoc = QJsonDocument::fromJson(QByteArray::fromStdString(message), &jsonParseError);
    if (jsonParseError.error != QJsonParseError::NoError || messageDoc.isEmpty()) {
        qWarning() << Q_FUNC_INFO << "Failed to obtain message from shell!: " << QString::fromStdString(message);
        return "";
    }

    QJsonObject retObj;
    retObj["Code"] = 0;
    retObj["Message"] = "Success";

    QJsonObject msgObj = messageDoc.object();
    QString cmdType = msgObj.value("CmdType").toString();
    QJsonObject data = msgObj.value("Data").toObject();
    qInfo() << "Cmd type: " << cmdType;
    if (cmdType == "CurrentUserChanged") {
        m_userName = data.value("Name").toString();
        qDebug() << "Current user changed, user name: " << m_userName;
    } else if (cmdType == "GetConfigs") {
        QJsonObject retDataObj;
        retDataObj["ShowAvatar"] = false;
        retDataObj["ShowUserName"] = false;
        retDataObj["ShowSwitchButton"] = false;
        retDataObj["ShowLockButton"] = false;
        retDataObj["DefaultAuthLevel"] = DefaultAuthLevel::StrongDefault;
        qInfo() << Q_FUNC_INFO << "m_appType " << m_appType;
        if(m_appType == AppType::Lock){
            retDataObj["AuthType"] = AuthType::AT_Fingerprint;
        }else if (m_appType == AppType::Login){
            retDataObj["AuthType"] = AuthType::AT_Custom;
        }

        retObj["Data"] = retDataObj;
    }

    QJsonDocument doc;
    doc.setObject(retObj);
    return doc.toJson().toStdString();
}

void LoginModule::slotIdentifyStatus(const QString &name, const int errorCode, const QString &msg)
{
    qDebug() << Q_FUNC_INFO << "LoginModule name :" << name << "\n error code:" << errorCode << " \n error msg:" << msg;
    m_isAcceptSignal = true;

    if (errorCode == 0) {
        QTimer::singleShot(500, this, [this, name] {
            qInfo() << Q_FUNC_INFO << "singleShot verify";
            AuthCallbackData data;
            data.account = name.toStdString();
            data.token = "";
            data.result = AuthResult::Success;
            messageCallback(data);
        });
    } else {
        // 发送一键登录失败的信息
        qWarning() << Q_FUNC_INFO << "slotIdentifyStatus recive failed";
        sendAuthTypeToSession();

        AuthCallbackData data;
        data.result = AuthResult::Failure;
        data.message = QString::number(errorCode).toStdString();
        messageCallback(data);
    }
}

void LoginModule::messageCallback(AuthCallbackData& data)
{
    if (!m_callbackFun) {
        qDebug() << Q_FUNC_INFO << "m_callbackFun is null";
        return;
    }

    if (m_spinner) {
        m_spinner->stop();
    }
    m_callbackFun(&data, m_callback->app_data);
}

void LoginModule::slotPrepareForSleep(bool active)
{
    qInfo() << Q_FUNC_INFO << active;

    // 发送是否休眠待机的信息
    if (active)
        sendAuthTypeToSession();
}

void LoginModule::sendAuthTypeToSession()
{
    qInfo() << Q_FUNC_INFO << "sendAuthTypeToSession";
    if (!m_messageCallbackFunc)
        return;

    if (m_spinner) {
        m_spinner->stop();
    }
    QJsonObject message;
    message.insert("CmdType", "setAuthTypeInfo");
    QJsonObject retDataObj;
    retDataObj["AuthType"] = AuthType::AT_Fingerprint;
    message["Data"] = retDataObj;
    QJsonDocument doc;
    doc.setObject(message);
    std::string ret = m_messageCallbackFunc(doc.toJson().toStdString(), m_callback->app_data);
    // 解析返回值
    QJsonParseError jsonParseError;
    const QJsonDocument retDoc = QJsonDocument::fromJson(QByteArray::fromStdString(ret), &jsonParseError);
    if (jsonParseError.error != QJsonParseError::NoError || retDoc.isEmpty()) {
        qWarning() << Q_FUNC_INFO << "Failed to analysis SlotPrepareForSleep info from shell!: " << QString::fromStdString(ret);
    }
}

} // namespace module
} // namespace dss