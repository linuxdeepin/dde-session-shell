// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UPDATECTRL_H
#define UPDATECTRL_H

#include "updatemodel.h"

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

using UpdateInter = com::deepin::lastore::Updater;
using JobInter = com::deepin::lastore::Job;
using ManagerInter = com::deepin::lastore::Manager;
using RecoveryInter = com::deepin::ABRecovery;
using PowerInter = com::deepin::system::Power;

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

    void doDistUpgrade();
    void doBackup();
    void doAction(UpdateModel::UpdateAction action);
    void startUpdateProgress();
    bool checkPower();
    void enableShortcuts(bool enable);

Q_SIGNALS:
    void requestDoPowerAction(bool isReboot);
    void requestExitUpdating();

private:
    explicit UpdateWorker(QObject *parent = nullptr);
    void init();
    UpdateModel::UpdateError analyzeJobErrorMessage(QString jobDescription);
    void fixError();

private slots:
    void onJobListChanged(const QList<QDBusObjectPath> &jobs);
    void onDistUpgradeStatusChanged(const QString &status);

private:
    PowerInter *m_powerInter;           // 电源
    ManagerInter *m_managerInter;       // 更新
    RecoveryInter *m_abRecoveryInter;   // 备份
    JobInter *m_distUpgradeJob;         // 更新job
    JobInter *m_fixErrorJob;           // 修复错误job
    bool m_fixErrorResult;              // 修复错误结果
};

#endif // UPDATECTRL_H
