// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "updateworker.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVariant>

#include <DDBusSender>

UpdateWorker::UpdateWorker(QObject* parent)
    : QObject(parent)
{

}

void UpdateWorker::doUpdate(bool powerOff)
{
    qInfo() << "Start update progress";

    // 检查电量
    const bool powerIsOK = checkPower();
    if (!powerIsOK) {
        Q_EMIT requestExitUpdating();
        DDBusSender()
            .service("org.freedesktop.Notifications")
            .path("/org/freedesktop/Notifications")
            .interface("org.freedesktop.Notifications")
            .method(QString("Notify"))
            .arg(tr("Update"))
            .arg(static_cast<uint>(0))
            .arg(QString(""))
            .arg(QString(""))
            .arg(tr("Please plug in and then install updates."))
            .arg(QStringList())
            .arg(QVariantMap())
            .arg(5000)
            .call();

        qWarning() << "Low power, exit update progress";
        return;
    }

    qInfo() << "Power check passed";

    prepareFullScreenUpgrade(powerOff);
}

bool UpdateWorker::checkPower()
{
    qInfo() << "Check power";
    PowerInter powerInter("com.deepin.system.Power", "/com/deepin/system/Power", QDBusConnection::systemBus(), this);
    bool onBattery = powerInter.onBattery();
    if (!onBattery) {
        qInfo() << "No battery";
        return true;
    }

    double data = powerInter.batteryPercentage();
    int batteryPercentage = uint(qMin(100.0, qMax(0.0, data)));
    qInfo() << "Battery percentage: " << batteryPercentage;
    return batteryPercentage >= 60;
}

void UpdateWorker::prepareFullScreenUpgrade(bool powerOff)
{
    qInfo() << "Do update after restart lightdm";

    QJsonObject content;
    QDBusInterface managerInter("com.deepin.lastore",
        "/com/deepin/lastore",
        "com.deepin.lastore.Manager",
        QDBusConnection::systemBus());
    content["DoUpgradeMode"] = managerInter.property("CheckUpdateMode").toInt();
    content["IsPowerOff"] = powerOff;

    QJsonDocument jsonDoc;
    jsonDoc.setObject(content);

    const QString& arg = jsonDoc.toJson();
    qInfo() << "Call function `PrepareFullScreenUpgrade` with arg:" << arg;

    QDBusPendingReply<QDBusObjectPath> reply = managerInter.asyncCall("PrepareFullScreenUpgrade", arg);
    QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(reply, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [reply, watcher] {
        watcher->deleteLater();
        if (reply.isValid()) {
            qInfo() << "Call `PrepareFullScreenUpgrade` function successfully, quit myself";
        } else {
            qWarning() << "Call `PrepareFullScreenUpgrade` function failed, error:" << reply.error().message();
        }
    });
}
