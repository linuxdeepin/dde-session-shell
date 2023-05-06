// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UPDATECTRL_H
#define UPDATECTRL_H

#include "updatemodel.h"
#include "dbuslogin1manager.h"

#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusMetaType>
#include <QtDBus/QtDBus>

#include <com_deepin_abrecovery.h>
#include <com_deepin_lastore_job.h>
#include <com_deepin_lastore_jobmanager.h>
#include <com_deepin_lastore_updater.h>
#include <com_deepin_system_systempower.h>
#include <com_deepin_sessionmanager.h>
#include <org_freedesktop_login1_session_self.h>
#include <org_freedesktop_dbus.h>

using UpdateInter = com::deepin::lastore::Updater;
using JobInter = com::deepin::lastore::Job;
using ManagerInter = com::deepin::lastore::Manager;
using RecoveryInter = com::deepin::ABRecovery;
using PowerInter = com::deepin::system::Power;
using SessionManagerInter = com::deepin::SessionManager;
using Login1SessionSelf = org::freedesktop::login1::Session;
using DBusManager = org::freedesktop::DBus;

class UpdateWorker : public QObject
{
    Q_OBJECT

public:
    static UpdateWorker *instance() {
        static UpdateWorker *pIns = nullptr;
        if (!pIns) {
            pIns = new UpdateWorker();
        }
        return pIns;
    };

    void doDistUpgrade(bool doBackup);
    void doDistUpgradeIfCanBackup();
    void doAction(UpdateModel::UpdateAction action);
    void startUpdateProgress();
    bool checkPower();
    void enableShortcuts(bool enable);
    void doPowerAction(bool reboot);
    void setLocked(const bool locked);

Q_SIGNALS:
    void requestExitUpdating();

private:
    explicit UpdateWorker(QObject *parent = nullptr);
    void init();
    UpdateModel::UpdateError analyzeJobErrorMessage(QString jobDescription);
    void fixError();
    void checkStatusAfterSessionActive();
    bool syncStartService(DBusExtendedAbstractInterface *interface);
    void createDistUpgradeJob(const QString& jobPath);
    void cleanLaStoreJob(QPointer<JobInter> dbusJob);

private slots:
    void onJobListChanged(const QList<QDBusObjectPath> &jobs);
    void onDistUpgradeStatusChanged(const QString &status);

private:
    PowerInter *m_powerInter;
    ManagerInter *m_managerInter;
    RecoveryInter *m_abRecoveryInter;
    SessionManagerInter *m_sessionManagerInter;
    DBusLogin1Manager *m_login1Manager;
    Login1SessionSelf* m_login1SessionSelf;
    JobInter *m_distUpgradeJob;     // 更新job
    JobInter *m_fixErrorJob;        // 修复错误job
    bool m_fixErrorResult;          // 修复错误结果
};

#endif // UPDATECTRL_H
