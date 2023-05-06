// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UPDATEMODEL_H
#define UPDATEMODEL_H

#include <QMap>
#include <QObject>
#include <QPair>
#include <QStringList>

class UpdateModel : public QObject
{
    Q_OBJECT

public:

enum UpdateStatus {
    Default = 0,
    Ready,
    PrepareFailed,
    BackingUp,
    BackupSuccess,
    BackupFailed,
    Installing,
    InstallSuccess,
    InstallFailed,
    InstallComplete
};
Q_ENUM(UpdateStatus)

enum UpdateError {
    NoError,
    LowPower,
    BackupInterfaceError,
    CanNotBackup,
    BackupNoSpace,
    BackupFailedUnknownReason,
    /* !! 以下失败是要重启电脑的，添加错误的时候请注意 */
    UpdateInterfaceError,
    InstallNoSpace,
    DependenciesBrokenError,
    DpkgInterrupted,
    UnKnown
};
Q_ENUM(UpdateError)

enum UpdateAction {
    None,
    DoBackupAgain,
    ExitUpdating,
    ContinueUpdating,
    FixError,
    Reboot,
    ShutDown
};
Q_ENUM(UpdateAction)

public:
    static UpdateModel *instance();

    void setUpdateStatus(UpdateModel::UpdateStatus status);
    UpdateStatus updateStatus() const { return m_updateStatus; }

    void setDistUpgradeProgress(double progress);
    double distUpgradeProgress() const { return m_distUpgradeProgress; }

    void setUpdateError(UpdateError error);
    UpdateError updateError() const { return m_updateError; };
    static QPair<QString, QString> updateErrorMessage(UpdateError error);

    void setLastErrorLog(const QString &log);
    QString lastErrorLog() const { return m_lastErrorLog; }

    void setBackupConfigValidation(bool valid);
    bool isBackupConfigValid() const { return m_isBackupConfigValid; }

    bool isUpdating() { return m_isUpdating; }
    void setIsUpdating(bool isUpdating) { m_isUpdating = isUpdating; }

    void setIsReboot(bool isReboot) { m_isReboot = isReboot; }
    bool isReboot() { return m_isReboot; }

    static QString updateActionText(UpdateAction action);

private:
    explicit UpdateModel(QObject *parent = nullptr);

signals:
    void updateAvailableChanged(bool available);
    void updateStatusChanged(UpdateStatus status);
    void distUpgradeProgressChanged(double progress);

private:
    bool m_updateAvailable;
    UpdateStatus m_updateStatus;
    double m_distUpgradeProgress;   // 更新进度
    UpdateError m_updateError;      // 错误类型
    QString m_lastErrorLog;         // 错误日志
    bool m_isBackupConfigValid;     // 可以备份的必要条件
    bool m_isUpdating;              // 是否正在更新
    bool m_isReboot;                // true 更新并重启 false 更新并关机
};

#endif // UPDATEMODEL_H
