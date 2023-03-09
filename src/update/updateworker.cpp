// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "updateworker.h"
#include "dconfig_helper.h"

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
    , m_sessionManagerInter(new SessionManagerInter("com.deepin.SessionManager", "/com/deepin/SessionManager", QDBusConnection::sessionBus(), this))
    , m_login1Manager(new DBusLogin1Manager("org.freedesktop.login1", "/org/freedesktop/login1", QDBusConnection::systemBus(), this))
    , m_login1SessionSelf(nullptr)
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
    connect(m_managerInter, &ManagerInter::serviceValidChanged, this, [this](bool valid) {
        if (!valid) {
            const auto status = UpdateModel::instance()->updateStatus();
            qWarning() << "Lastore daemon manager service is invalid, curren status: " << status;
            if (m_distUpgradeJob) {
                delete m_distUpgradeJob;
                m_distUpgradeJob = nullptr;
            }

            if (status != UpdateModel::Installing)
                return;

            UpdateModel::instance()->setLastErrorLog("com.deepin.lastore.Manager interface is invalid.");
            UpdateModel::instance()->setUpdateError(UpdateModel::UpdateError::UpdateInterfaceError);
            UpdateModel::instance()->setUpdateStatus(UpdateModel::UpdateStatus::InstallFailed);
        }
    });
    connect(m_abRecoveryInter, &RecoveryInter::JobEnd, this, [this](const QString &kind, bool success, const QString &errMsg) {
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
    connect(m_abRecoveryInter, &RecoveryInter::BackingUpChanged, this, [](bool value) {
        qInfo() << "Backing up changed: " << value;
        if (value) {
            UpdateModel::instance()->setUpdateStatus(UpdateModel::UpdateStatus::BackingUp);
        }
    });
    connect(m_abRecoveryInter, &RecoveryInter::ConfigValidChanged, this, [](bool value) {
        qInfo() << "Backup config valid changed: " << value;
        UpdateModel::instance()->setBackupConfigValidation(value);
    });
    connect(m_abRecoveryInter, &RecoveryInter::serviceValidChanged, this, [](bool valid) {
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

    if (m_login1Manager->isValid()) {
        QString session_self = m_login1Manager->GetSessionByPID(0).value().path();
        m_login1SessionSelf = new Login1SessionSelf("org.freedesktop.login1", session_self, QDBusConnection::systemBus(), this);
        m_login1SessionSelf->setSync(false);
        if (m_login1SessionSelf->isValid()) {
            // 处理场景：开始更新切tty，当前session被冻结，过一段时间后切换回来，此时以及安装完成，dist_upgrade job已经退出
            connect(m_login1SessionSelf, &Login1SessionSelf::ActiveChanged, this, [this](bool active) {
                qInfo() << "UpdateWorker: login1 self session active changed:" << active;
                if (UpdateModel::instance()->updateStatus() == UpdateModel::Installing) {
                    const int lastoreDaemonStatus = DConfigHelper::instance()->getConfig("org.deepin.lastore", "org.deepin.lastore", "", "lastore-daemon-status", 0).toInt();
                    qInfo() << "Lastore daemon status: " << lastoreDaemonStatus;
                    static const int IS_UPDATE_READY = 1; // 第一位表示更新是否可用
                    const bool isUpdateReady = lastoreDaemonStatus & IS_UPDATE_READY;
                    if (!isUpdateReady) {
                        // 更新成功
                        qInfo() << "Update successfully";
                        UpdateModel::instance()->setUpdateStatus(UpdateModel::InstallSuccess);
                    } else {
                        checkStatusAfterSessionActive();
                    }
                }
            });
        } else {
            qWarning() << "Login1 self session is invalid";
        }
    } else {
        qWarning() << "Login1 manager is invalid, error: " << m_login1Manager->lastError().type();
    }
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

    if (UpdateModel::instance()->updateStatus() >= UpdateModel::Installing) {
        qWarning() << "Installing, won't do it again";
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
        qInfo() << "Job id : " << id;
        if (id == "dist_upgrade" && m_distUpgradeJob == nullptr) {
            qInfo() << "Create dist upgrade job";
            m_distUpgradeJob = new JobInter("com.deepin.lastore", jobPath, QDBusConnection::systemBus(), this);
            connect(m_distUpgradeJob, &__Job::ProgressChanged, UpdateModel::instance(), &UpdateModel::setDistUpgradeProgress);
            connect(m_distUpgradeJob, &__Job::StatusChanged, this, &UpdateWorker::onDistUpgradeStatusChanged);
            UpdateModel::instance()->setDistUpgradeProgress(m_distUpgradeJob->progress());
            onDistUpgradeStatusChanged(m_distUpgradeJob->status());
        }
    }
}

void UpdateWorker::onDistUpgradeStatusChanged(const QString &status)
{
    // 无需处理ready状态
    static const QMap<QString, UpdateModel::UpdateStatus> DIST_UPGRADE_STATUS_MAP = {
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
            doPowerAction(true);
            break;
        case UpdateModel::ShutDown:
            doPowerAction(false);
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
    connect(m_fixErrorJob, &JobInter::StatusChanged, this, [this](const QString status) {
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

void UpdateWorker::doPowerAction(bool reboot)
{
    qInfo() << "UpdateWorker do power action, is reboot: " << reboot;

    if (m_login1Manager->isValid()) {
        QDBusPendingReply<InhibitorsList> reply = m_login1Manager->ListInhibitors();
        reply.waitForFinished();
        if (reply.isValid()) {
            const auto &value = reply.value();
            qInfo() << "Inhibitors size: " << value.size();
            for (const Inhibit &inhibit : value) {
                qInfo() << "who:" << inhibit.who;
                qInfo() << "why:" << inhibit.why;
                qInfo() << "pid:" << inhibit.pid;
                qInfo() << "mode:" << inhibit.mode;
            }
        } else {
            qWarning() << "Get inhibitors failed, error: " << reply.error().message();
        }
    } else {
        qWarning() << "Login1 manager is invalid";
    }

    auto powerActionReply = reboot ? m_sessionManagerInter->RequestReboot() : m_sessionManagerInter->RequestShutdown();
    powerActionReply.waitForFinished();
    if (powerActionReply.isError()) {
        qWarning() << "Do power action failed: " << powerActionReply.error().message();
    } else {
        qInfo() << "Do power action successfully ";
    }
}

void UpdateWorker::enableShortcuts(bool enable)
{
    qInfo() << "Enable shortcuts: " << enable;
    QDBusPendingCall reply = DDBusSender()
        .service("com.deepin.dde.osd")
        .path("/")
        .interface("com.deepin.dde.osd")
        .property("OSDEnabled")
        .set(enable);

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [reply, watcher] {
        watcher->deleteLater();
        if (!reply.isValid()) {
            qWarning() << "Set `OSDEnabled` property failed, error: " << reply.error().message();
        }
    });
}

/**
 * @brief 同步调用的方式拉起服务
 *
 * @param interface 需要拉起的服务
 * @return true service is valid
 * @return false service is invalid
 */
bool UpdateWorker::syncStartService(DBusExtendedAbstractInterface *interface)
{
    const QString &service = interface->service();
    DBusManager dbusManager("org.freedesktop.DBus", "/", QDBusConnection::systemBus());
    QDBusReply<uint32_t> reply = dbusManager.call("StartServiceByName", service, quint32(0));
    if (reply.isValid()) {
        qInfo() << QString("Start %1 service result: ").arg(service) << reply.value();
        return reply.value() == 1;
    } else {
        qWarning() << QString("Start %1 service failed, error: ").arg(service) << reply.error().message();
        return false;
    }
}

/**
 * @brief 此函数用来在切换session后检查备份和更新的状态
 * 避免用户切换到其他的tty再切换回来无法正确获取状态导致进度卡住
 *
 */
void UpdateWorker::checkStatusAfterSessionActive()
{
    qInfo() << "Check installation status";
    if (!m_managerInter->isValid() && m_distUpgradeJob) {
        delete m_distUpgradeJob;
        m_distUpgradeJob = nullptr;
        syncStartService(m_managerInter);
    }

    if (m_managerInter->isValid()) {
        onJobListChanged(m_managerInter->jobList());
        if (m_distUpgradeJob) {
            return;
        }
    }

    qInfo() << "Check backup status";
    if (m_abRecoveryInter->isValid() || syncStartService(m_abRecoveryInter) ) {
        QDBusReply<bool> reply = m_abRecoveryInter->call("CanBackup");
        if (!reply.isValid()) {
            qWarning() << "Call `CanBackup` function failed, error: " << reply.error().message();
            return;
        }

        if (m_abRecoveryInter->backingUp()) {
            qInfo() << "Backing up";
            return;
        }

        if (!m_abRecoveryInter->hasBackedUp()) {
            qWarning() << "System can backup but do not has backed up";
            UpdateModel::instance()->setUpdateError(UpdateModel::BackupFailedUnknownReason);
            UpdateModel::instance()->setUpdateStatus(UpdateModel::BackupFailed);
            return;
        }

        // 已经备份完成但是并没有安装则开始安装
        if (UpdateModel::instance()->updateStatus() < UpdateModel::Installing) {
            doDistUpgrade();
        }
    }
}
