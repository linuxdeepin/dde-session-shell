// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef __UPDATEMODEL_H__
#define __UPDATEMODEL_H__

#include "updatemodel.h"

#include <QDebug>
#include <QMetaEnum>
#include <QList>
#include <QMap>

UpdateModel::UpdateModel(QObject *parent)
    : QObject{parent}
    , m_updateAvailable(false)
    , m_updateMode(UpdateModel::UpdateType::Invalid)
    , m_updateStatus(UpdateStatus::Default)
    , m_distUpgradeProgress(0)
    , m_isBackupConfigValid(true)
    , m_isUpdating(false)
    , m_isReboot(false)
{

}

UpdateModel* UpdateModel::instance()
{
    static UpdateModel *updateModelInstance = nullptr;
    // 对于单线程来讲，安全性足够
    if (!updateModelInstance) {
        updateModelInstance = new UpdateModel();
    }

    return updateModelInstance;
}

void UpdateModel::setUpdateStatus(UpdateModel::UpdateStatus status)
{
    qInfo() << "UpdateModel::setUpdateStatus: " << status << ", current: " << m_updateStatus;
    if (m_updateStatus == status)
        return;

    m_updateStatus = status;
    Q_EMIT updateStatusChanged(m_updateStatus);
}

void UpdateModel::setDistUpgradeProgress(double progress)
{
    qDebug() << "UpdateModel::setDistUpgradeProgress: " << progress << ", current: " << m_distUpgradeProgress;
    if (qFuzzyCompare(progress, m_distUpgradeProgress))
        return;

    m_distUpgradeProgress = progress;
    Q_EMIT distUpgradeProgressChanged(m_distUpgradeProgress);
}

void UpdateModel::setUpdateError(UpdateError error)
{
    qWarning() << "UpdateModel::setUpdateError: " << error << ", current: " << m_updateError;
    if (m_updateError == error)
        return;

    m_updateError = error;
}

QPair<QString, QString> UpdateModel::updateErrorMessage(UpdateError error)
{
    static const QMap<UpdateError, QPair<QString, QString>> ErrorMessage = {
        {UpdateError::UnKnown, qMakePair(tr("Update failed"), tr("Unknown error"))},
        {UpdateError::CanNotBackup, qMakePair(tr("Backup failed"), tr("Unable to perform system backup. If you continue the updates, you cannot roll back to the old system later."))},
        {UpdateError::BackupNoSpace, qMakePair(tr("Backup failed"), QString(tr("Insufficient disk space. Please clean up your disk and try again.")))},
        {UpdateError::BackupInterfaceError, qMakePair(tr("Backup failed"), tr("Failed to connect to backup services. Please check and retry again."))},
        {UpdateError::BackupFailedUnknownReason, qMakePair(tr("Backup failed"), tr("Unknown error"))},
        {UpdateError::UpdateInterfaceError, qMakePair(tr("Update failed"), tr("Failed to connect to update services. Please check and retry again."))},
        {UpdateError::InstallNoSpace, qMakePair(tr("Update failed"), tr("Insufficient disk space. Please clean up your disk and try again."))},
        {UpdateError::DependenciesBrokenError, qMakePair(tr("Update failed"), tr("Dependency error"))},
        {UpdateError::DpkgInterrupted, qMakePair(tr("Update failed"), tr("DPKG error"))}
    };

    if (ErrorMessage.contains(error))
        return ErrorMessage.value(error);

    return qMakePair(QString(), QString());
}

void UpdateModel::setLastErrorLog(const QString &log)
{
    m_lastErrorLog = log;
}

void UpdateModel::setBackupConfigValidation(bool valid)
{
    m_isBackupConfigValid = valid;
}

QString UpdateModel::updateActionText(UpdateAction action)
{
    static const QMap<UpdateAction, QString> ActionsText = {
        {None, tr("")},
        {DoBackupAgain, tr("Back Up Again")},
        {ExitUpdating, tr("Abort")},
        {ContinueUpdating, tr("Proceed to Update")},
        {Reboot, tr("Reboot")},
        {ShutDown, tr("Shut Down")}
    };

    return ActionsText.value(action);
}


#endif // __UPDATEMODEL_H__