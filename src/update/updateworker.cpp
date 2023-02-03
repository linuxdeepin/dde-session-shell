// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "updateworker.h"

#include <QTimer>
#include <QDebug>
#include <QDir>
#include <QDBusPendingReply>
#include <QDBusMetaType>
#include <QMap>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVariant>
#include <QFont>
#include <QWidget>
#include <QApplication>

#include <DDBusSender>

UpdateWorker::UpdateWorker(QObject *parent)
    : QObject(parent)
    , m_powerInter(new PowerInter("com.deepin.system.Power", "/com/deepin/system/Power", QDBusConnection::systemBus(), this))
    , m_managerInter(new ManagerInter("com.deepin.lastore", "/com/deepin/lastore", QDBusConnection::systemBus(), this))
    , m_abRecoveryInter(new RecoveryInter("com.deepin.ABRecovery", "/com/deepin/ABRecovery", QDBusConnection::systemBus(), this))
    , m_distUpgradeJob(nullptr)
    , m_fixErrorJob(nullptr)
{
    init();
}

void UpdateWorker::init()
{
    m_managerInter->setSync(false);
    m_abRecoveryInter->setSync(false);
    connect(m_managerInter, &ManagerInter::JobListChanged, this, &UpdateWorker::onJobListChanged);
    connect(m_managerInter, &ManagerInter::serviceValidChanged, this, [] (bool valid) {
        if (!valid) {
            const auto status = UpdateModel::instance()->updateStatus();
            qWarning() << "Lastore daemon manager service is invalid, curren status: " << status;
            if (status != UpdateModel::Installing)
                return;

            UpdateModel::instance()->setLastErrorLog("com.deepin.lastore.Manager interface is invalid.");
            UpdateModel::instance()->setUpdateError(UpdateModel::UpdateError::UpdateInterfaceError);
            UpdateModel::instance()->setUpdateStatus(UpdateModel::UpdateStatus::InstallFailed);
        }
    });
    connect(m_abRecoveryInter, &RecoveryInter::JobEnd, this, [this] (const QString &kind, bool success, const QString &errMsg) {
        qInfo() << "Backup job end, kind: " << kind << ", success: " << success << ", error message: " << errMsg;
        if ("backup" != kind) {
            qWarning() << "Kind error: " << kind;
            return;
        }

        if (success) {
            UpdateModel::instance()->setUpdateStatus(UpdateModel::UpdateStatus::BackupSuccess);
            doDistUpgrade();
        } else {
            UpdateModel::instance()->setLastErrorLog(errMsg);
            UpdateModel::instance()->setUpdateError(UpdateModel::UpdateError::BackupFailedUnknownReason);
            UpdateModel::instance()->setUpdateStatus(UpdateModel::UpdateStatus::BackupFailed);
        }
    });
    connect(m_abRecoveryInter, &RecoveryInter::BackingUpChanged, this, [] (bool value) {
        qInfo() << "Backing up changed: " << value;
        if (value) {
            UpdateModel::instance()->setUpdateStatus(UpdateModel::UpdateStatus::BackingUp);
        }
    });
    connect(m_abRecoveryInter, &RecoveryInter::ConfigValidChanged, this, [] (bool value) {
        qInfo() << "Backup config valid changed: " << value;
        UpdateModel::instance()->setBackupConfigValidation(value);
    });
    connect(m_abRecoveryInter, &RecoveryInter::serviceValidChanged, this, [] (bool valid) {
        if (!valid) {
            const auto status = UpdateModel::instance()->updateStatus();
            qWarning() << "AB recovery service was invalid, current status: " << status;
            if (status != UpdateModel::BackingUp)
                return;

            UpdateModel::instance()->setLastErrorLog("AB recovery service was invalid.");
            UpdateModel::instance()->setUpdateError(UpdateModel::UpdateError::BackupInterfaceError);
            UpdateModel::instance()->setUpdateStatus(UpdateModel::UpdateStatus::BackupFailed);
        }
    });
}

/**
 * @brief 开始更新流程
 * 一般流程： 检查电量 -> 备份 -> 安装 -> 重启/关机
 */
void UpdateWorker::startUpdateProgress()
{
    qInfo() << "Start update progress";

    // 检查电量
    const bool powerIsOK = checkPower();
    if (!powerIsOK) {
        UpdateModel::instance()->setUpdateStatus(UpdateModel::UpdateStatus::Default);
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
            .arg(tr("Please plug in the power before starting the update"))
            .arg(QStringList())
            .arg(QVariantMap())
            .arg(5000)
            .call();

        qWarning() << "Low power, exit update progress";
        return;
    }

    qInfo() << "Power check passed";

    // 备份; PS:收到备份完成的信号后自动进入安装过程
    doBackup();
}

void UpdateWorker::doDistUpgrade()
{
    qInfo() << "Do dist upgrade";
    if (!m_managerInter->isValid()) {
        UpdateModel::instance()->setLastErrorLog("com.deepin.lastore.Manager interface is invalid.");
        UpdateModel::instance()->setUpdateError(UpdateModel::UpdateError::UpdateInterfaceError);
        UpdateModel::instance()->setUpdateStatus(UpdateModel::UpdateStatus::InstallFailed);
        return;
    }

    QDBusPendingReply<QDBusObjectPath> reply = m_managerInter->DistUpgrade();
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [reply, watcher] {
        watcher->deleteLater();
        if (reply.isValid()) {
            UpdateModel::instance()->setUpdateStatus(UpdateModel::UpdateStatus::Installing);
        } else {
            const QString &errorMessage = watcher->error().message();
            qWarning() << "Do dist upgrade failed:" << watcher->error().message();
            UpdateModel::instance()->setLastErrorLog(errorMessage);
            UpdateModel::instance()->setUpdateError(UpdateModel::UpdateError::UnKnown);
            UpdateModel::instance()->setUpdateStatus(UpdateModel::UpdateStatus::InstallFailed);
        }
    });
}

void UpdateWorker::onJobListChanged(const QList<QDBusObjectPath> &jobs)
{
    qInfo() << "Job list changed";
    for (const auto &job : jobs) {
        const QString &jobPath = job.path();
        qInfo() << "path : " << jobPath;
        JobInter jobInter("com.deepin.lastore", jobPath, QDBusConnection::systemBus());
        if (!jobInter.isValid()) {
            qWarning() << "Job is invalid";
            continue;
        }

        // id maybe scrapped
        const QString &id = jobInter.id();
        qDebug() << "Job id : " << id;
        if (id == "dist_upgrade" && m_distUpgradeJob == nullptr) {
            m_distUpgradeJob = new JobInter("com.deepin.lastore", jobPath, QDBusConnection::systemBus(), this);
            connect(m_distUpgradeJob, &__Job::ProgressChanged, UpdateModel::instance(), &UpdateModel::setDistUpgradeProgress);
            connect(m_distUpgradeJob, &__Job::StatusChanged, this, &UpdateWorker::onDistUpgradeStatusChanged);
        }
    }
}

void UpdateWorker::onDistUpgradeStatusChanged(const QString &status)
{
    static const QMap<QString, UpdateModel::UpdateStatus> DIST_UPGRADE_STATUS_MAP = {
        {"ready", UpdateModel::UpdateStatus::Ready},
        {"running", UpdateModel::UpdateStatus::Installing},
        {"failed", UpdateModel::UpdateStatus::InstallFailed},
        {"succeed", UpdateModel::UpdateStatus::InstallSuccess},
        {"end", UpdateModel::UpdateStatus::InstallComplete}
    };

    qInfo() << "Dist upgrade status changed " << status;
    if (DIST_UPGRADE_STATUS_MAP.contains(status)) {
        const UpdateModel::UpdateStatus updateStatus = DIST_UPGRADE_STATUS_MAP.value(status);
        if (updateStatus == UpdateModel::UpdateStatus::InstallComplete) {
            m_managerInter->CleanJob(m_distUpgradeJob->id());
            delete m_distUpgradeJob;
            m_distUpgradeJob = nullptr;
        } else {
            if (updateStatus == UpdateModel::UpdateStatus::InstallFailed && m_distUpgradeJob) {
                UpdateModel::instance()->setLastErrorLog(m_distUpgradeJob->description());
                UpdateModel::instance()->setUpdateError(analyzeJobErrorMessage(m_distUpgradeJob->description()));
                UpdateModel::instance()->setUpdateStatus(UpdateModel::InstallFailed);
            }
            UpdateModel::instance()->setUpdateStatus(updateStatus);
        }
    } else {
        qWarning() << "Unknown dist upgrade status";
        UpdateModel::instance()->setUpdateStatus(UpdateModel::UpdateStatus::Unknown);
    }
}

UpdateModel::UpdateError UpdateWorker::analyzeJobErrorMessage(QString jobDescription)
{
    qInfo() << "Analyze job error message: " << jobDescription;
    QJsonParseError err_rpt;
    QJsonDocument jobErrorMessage = QJsonDocument::fromJson(jobDescription.toUtf8(), &err_rpt);

    if (err_rpt.error != QJsonParseError::NoError) {
        qWarning() << "Json format error";
        return UpdateModel::UpdateError::UnKnown;
    }
    const QJsonObject &object = jobErrorMessage.object();
    QString errorType =  object.value("ErrType").toString();

    if (errorType.contains("unmetDependencies", Qt::CaseInsensitive) || errorType.contains("dependenciesBroken", Qt::CaseInsensitive)) {
        return UpdateModel::UpdateError::DependenciesBrokenError;
    }
    if (errorType.contains("insufficientSpace", Qt::CaseInsensitive)) {
        return UpdateModel::UpdateError::InstallNoSpace;
    }
    if (errorType.contains("interrupted", Qt::CaseInsensitive)) {
        return UpdateModel::UpdateError::DpkgInterrupted;
    }

    return UpdateModel::UpdateError::UnKnown;
}

void UpdateWorker::doBackup()
{
    qInfo() << "Prepare to do backup";
    QDBusPendingCall call = m_abRecoveryInter->CanBackup();
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, call] {
        if (!call.isError()) {
            QDBusReply<bool> reply = call.reply();
            const bool value = reply.value();
            if (value) {
                qInfo() << "Start backup";
                UpdateModel::instance()->setUpdateStatus(UpdateModel::BackingUp);
                m_abRecoveryInter->StartBackup();
            } else {
                qWarning() << "Can not backup";
                UpdateModel::instance()->setLastErrorLog(reply.error().message());
                UpdateModel::instance()->setUpdateError(UpdateModel::CanNotBackup);
                UpdateModel::instance()->setUpdateStatus(UpdateModel::BackupFailed);
            }
        } else {
            qWarning() << "Call `CanBackup` failed";
            UpdateModel::instance()->setLastErrorLog("Call `CanBackup` method in dbus interface - com.deepin.ABRecovery failed");
            UpdateModel::instance()->setUpdateError(UpdateModel::UpdateInterfaceError);
            UpdateModel::instance()->setUpdateStatus(UpdateModel::UpdateStatus::BackupFailed);
        }
    });
}

void UpdateWorker::doAction(UpdateModel::UpdateAction action)
{
    qInfo() << "Do action: " << action;
    switch (action) {
        case UpdateModel::DoBackupAgain:
            startUpdateProgress();
            break;
        case UpdateModel::ContinueUpdating:
            doDistUpgrade();
            break;
        case UpdateModel::ExitUpdating:
        case UpdateModel::CancelUpdating:
            Q_EMIT requestExitUpdating();
            break;
        case UpdateModel::Reboot:
            Q_EMIT requestDoPowerAction(true);
            break;
        case UpdateModel::ShutDown:
            Q_EMIT requestDoPowerAction(false);
            break;
        case UpdateModel::FixError:
            // TODO 交互逻辑和UI需要重新设计
            fixError();
            break;
        default:
            break;
    }
}

bool UpdateWorker::checkPower()
{
    qInfo() << "Check power";
    bool onBattery = m_powerInter->onBattery();
    if (!onBattery) {
        qInfo() << "No battery";
        return true;
    }

    double data = m_powerInter->batteryPercentage();
    int batteryPercentage = uint(qMin(100.0, qMax(0.0, data)));
    qInfo() << "Battery percentage: " << batteryPercentage;
    return batteryPercentage >= 60;
}

/**
 * @brief 修复更新错误（现在只能修复dpkg错误）
 *
 */
void UpdateWorker::fixError()
{
    qInfo() << "UpdateWorker::fixError: " << UpdateModel::instance()->updateError();
    if (m_fixErrorJob) {
        qWarning() << "Fix error job is not nullptr, won't fix error again";
        return;
    }

    if(UpdateModel::instance()->updateError() != UpdateModel::DpkgInterrupted) {
        qWarning() << "Only support fixing `dpkgInterrupted` error now";
        return;
    }

    QDBusReply<QDBusObjectPath> reply = m_managerInter->call("FixError", "dpkgInterrupted");
    if (!reply.isValid()) {
        qWarning() << "Call `FixError` reply is invalid, error: " << reply.error().message();
        return;
    }

    QString path = reply.value().path();
    m_fixErrorJob = new JobInter("com.deepin.lastore", path, QDBusConnection::systemBus());
    connect(m_fixErrorJob, &JobInter::StatusChanged, this, [ = ] (const QString status) {
        qInfo() << "Fix error job status changed :" << status;
        if (status == "succeed" || status == "failed" || status == "end") {
            if (m_fixErrorJob) {
                m_managerInter->CleanJob(m_fixErrorJob->id());
                m_fixErrorJob->deleteLater();
                m_fixErrorJob = nullptr;
            }

            if (status == "succeed") {
                m_fixErrorResult = false;
                startUpdateProgress();
            } else if(status == "failed") {
                m_fixErrorResult = false;
            }
        }
    });
}

