/*
 * Copyright (C) 2015 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     sbw <sbw@sbw.so>
 *             kirigaya <kirigaya@mkacg.com>
 *             Hualet <mr.asianwang@gmail.com>
 *
 * Maintainer: sbw <sbw@sbw.so>
 *             kirigaya <kirigaya@mkacg.com>
 *             Hualet <mr.asianwang@gmail.com>
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

#include "shutdownwidget.h"
#include "multiuserswarningview.h"

#if 0 // storage i10n
QT_TRANSLATE_NOOP("ShutdownWidget", "Shut down"),
                  QT_TRANSLATE_NOOP("ShutdownWidget", "Reboot"),
                  QT_TRANSLATE_NOOP("ShutdownWidget", "Suspend"),
                  QT_TRANSLATE_NOOP("ShutdownWidget", "Hibernate")
#endif

ShutdownWidget::ShutdownWidget(QWidget *parent)
    : QFrame(parent)
    , m_index(-1)
    , m_model(nullptr)
    , m_systemMonitor(nullptr)
    , m_login1Inter(new DBusLogin1Manager("org.freedesktop.login1", "/org/freedesktop/login1", QDBusConnection::systemBus(), this))
    , m_switchosInterface(new HuaWeiSwitchOSInterface("com.huawei", "/com/huawei/switchos", QDBusConnection::sessionBus(), this))
{
    m_frameDataBind = FrameDataBind::Instance();

    m_inhibitorBlacklists << "NetworkManager" << "ModemManager" << "com.deepin.daemon.Power";

    initUI();
    initConnect();

    std::function<void (QVariant)> function = std::bind(&ShutdownWidget::onOtherPageChanged, this, std::placeholders::_1);
    int index = m_frameDataBind->registerFunction("ShutdownWidget", function);

    connect(this, &ShutdownWidget::destroyed, this, [ = ] {
        m_frameDataBind->unRegisterFunction("ShutdownWidget", index);
    });
}

void ShutdownWidget::initConnect()
{
    connect(m_requireRestartButton, &RoundItemButton::clicked, this, [ = ] {
        m_currentSelectedBtn = m_requireRestartButton;
        onRequirePowerAction(SessionBaseModel::PowerAction::RequireRestart, false);
    });
    connect(m_requireShutdownButton, &RoundItemButton::clicked, this, [ = ] {
        m_currentSelectedBtn = m_requireShutdownButton;
        onRequirePowerAction(SessionBaseModel::PowerAction::RequireShutdown, false);
    });
    connect(m_requireSuspendButton, &RoundItemButton::clicked, this, [ = ] {
        m_currentSelectedBtn = m_requireSuspendButton;
        onRequirePowerAction(SessionBaseModel::PowerAction::RequireSuspend, false);
    });
    connect(m_requireHibernateButton, &RoundItemButton::clicked, this, [ = ] {
        m_currentSelectedBtn = m_requireHibernateButton;
        onRequirePowerAction(SessionBaseModel::PowerAction::RequireHibernate, false);
    });
    connect(m_requireLockButton, &RoundItemButton::clicked, this, [ = ] {
        m_currentSelectedBtn = m_requireLockButton;
        onRequirePowerAction(SessionBaseModel::PowerAction::RequireLock, false);
    });
    connect(m_requireSwitchUserBtn, &RoundItemButton::clicked, this, [ = ] {
        m_currentSelectedBtn = m_requireSwitchUserBtn;
        onRequirePowerAction(SessionBaseModel::PowerAction::RequireSwitchUser, false);
    });
    if (m_requireSwitchSystemBtn) {
        connect(m_requireSwitchSystemBtn, &RoundItemButton::clicked, this, [ = ] {
            m_currentSelectedBtn = m_requireSwitchSystemBtn;
            onRequirePowerAction(SessionBaseModel::PowerAction::RequireSwitchSystem, false);
        });
    }
    connect(m_requireLogoutButton, &RoundItemButton::clicked, this, [ = ] {
        m_currentSelectedBtn = m_requireLogoutButton;
        onRequirePowerAction(SessionBaseModel::PowerAction::RequireLogout, false);
    });

    if (m_systemMonitor) {
        connect(m_systemMonitor, &SystemMonitor::clicked, this, &ShutdownWidget::runSystemMonitor);
    }
}

void ShutdownWidget::updateTr(RoundItemButton *widget, const QString &tr)
{
    m_trList << std::pair<std::function<void (QString)>, QString>(std::bind(&RoundItemButton::setText, widget, std::placeholders::_1), tr);
}

void ShutdownWidget::onOtherPageChanged(const QVariant &value)
{
    m_index = value.toInt();

    for (auto it = m_btnList.constBegin(); it != m_btnList.constEnd(); ++it) {
        (*it)->updateState(RoundItemButton::Normal);
    }

    m_currentSelectedBtn = m_btnList.at(m_index);
    m_currentSelectedBtn->updateState(RoundItemButton::Checked);
}

void ShutdownWidget::hideToplevelWindow()
{
    QWidgetList widgets = qApp->topLevelWidgets();
    for (QWidget *widget : widgets) {
        if (widget->isVisible()) {
            widget->hide();
        }
    }
}

bool ShutdownWidget::beforeInvokeAction(const SessionBaseModel::PowerAction action)
{
    const QList<InhibitWarnView::InhibitorData> inhibitors = listInhibitors(action);

    const QList<std::shared_ptr<User>> &loginUsers = m_model->logindUser();

    // change ui
    if (m_confirm || !inhibitors.isEmpty() || loginUsers.length() > 1) {
        for (RoundItemButton *btn : m_btnList) {
            btn->hide();
        }

        if (m_warningView != nullptr) {
            m_mainLayout->removeWidget(m_warningView);
            m_warningView->deleteLater();
            m_warningView = nullptr;
        }
    }

    if (!inhibitors.isEmpty()) {
        InhibitWarnView *view = new InhibitWarnView(action, this);
        view->setFocusPolicy(Qt::NoFocus);
        setFocusPolicy(Qt::NoFocus);
        view->setAction(action);
        view->setInhibitorList(inhibitors);

        switch (action) {
        case SessionBaseModel::PowerAction::RequireShutdown:
            view->setInhibitConfirmMessage(tr("The programs are preventing the computer from shutting down, and forcing shut down may cause data loss.") + "\n" +
                                           tr("To close the program, click Cancel, and then close the program."));
            break;
        case SessionBaseModel::PowerAction::RequireSwitchSystem:
        case SessionBaseModel::PowerAction::RequireRestart:
            view->setInhibitConfirmMessage(tr("The programs are preventing the computer from reboot, and forcing reboot may cause data loss.") + "\n" +
                                           tr("To close the program, click Cancel, and then close the program."));
            break;
        case SessionBaseModel::PowerAction::RequireSuspend:
            view->setInhibitConfirmMessage(tr("The programs are preventing the computer from suspend, and forcing suspend may cause data loss.") + "\n" +
                                           tr("To close the program, click Cancel, and then close the program."));
            break;
        case SessionBaseModel::PowerAction::RequireHibernate:
            view->setInhibitConfirmMessage(tr("The programs are preventing the computer from hibernate, and forcing hibernate may cause data loss.") + "\n" +
                                           tr("To close the program, click Cancel, and then close the program."));
            break;
        case SessionBaseModel::PowerAction::RequireLogout:
            view->setInhibitConfirmMessage(tr("The programs are preventing the computer from log out, and forcing log out may cause data loss.") + "\n" +
                                           tr("To close the program, click Cancel, and then close the program."));
            break;
        default:
            return {};
        }


        bool isAccept = true;
        for (auto inhib : inhibitors) {
            if (inhib.mode.compare("block") == 0) {
                isAccept = false;
                break;
            }
        }

        if (action == SessionBaseModel::PowerAction::RequireShutdown) {
            view->setAcceptReason(tr("Shut down"));
            view->setAcceptVisible(isAccept);
        } else if (action == SessionBaseModel::PowerAction::RequireRestart || action == SessionBaseModel::PowerAction::RequireSwitchSystem) {
            view->setAcceptReason(tr("Reboot"));
            view->setAcceptVisible(isAccept);
        } else if (action == SessionBaseModel::PowerAction::RequireSuspend)
            view->setAcceptReason(tr("Suspend"));
        else if (action == SessionBaseModel::PowerAction::RequireHibernate)
            view->setAcceptReason(tr("Hibernate"));
        else if (action == SessionBaseModel::PowerAction::RequireLogout) {
            view->setAcceptReason(tr("Log out"));
            view->setAcceptVisible(isAccept);
        }

        m_warningView = view;
        m_mainLayout->addWidget(m_warningView);
        m_mainLayout->setAlignment(m_warningView, Qt::AlignCenter);
        m_mainLayout->setCurrentWidget(m_warningView);

        connect(view, &InhibitWarnView::cancelled, this, &ShutdownWidget::onCancelAction);
        connect(view, &InhibitWarnView::actionInvoked, [this, action] {
            m_model->setPowerAction(action);
        });

        return false;
    }

    if (loginUsers.length() > 1 && (action == SessionBaseModel::PowerAction::RequireShutdown || action == SessionBaseModel::PowerAction::RequireRestart)) {
        QList<std::shared_ptr<User>> tmpList = loginUsers;

        for (auto it = tmpList.begin(); it != tmpList.end();) {
            if ((*it)->uid() == m_model->currentUser()->uid()) {
                it = tmpList.erase(it);
                continue;
            }
            ++it;
        }

        MultiUsersWarningView *view = new MultiUsersWarningView(this);
        view->setFocusPolicy(Qt::NoFocus);
        setFocusPolicy(Qt::NoFocus);
        view->setUsers(tmpList);
        view->setAction(action);
        if (action == SessionBaseModel::PowerAction::RequireShutdown)
            view->setAcceptReason(tr("Shut down"));
        else if (action == SessionBaseModel::PowerAction::RequireRestart)
            view->setAcceptReason(tr("Reboot"));

        m_warningView = view;
        m_mainLayout->addWidget(m_warningView);
        m_mainLayout->setAlignment(m_warningView, Qt::AlignCenter);
        m_mainLayout->setCurrentWidget(m_warningView);

        connect(view, &MultiUsersWarningView::cancelled, this, &ShutdownWidget::onCancelAction);
        connect(view, &MultiUsersWarningView::actionInvoked, [this, action] {
            m_model->setPowerAction(action);
        });

        return false;
    }

    if (m_confirm && (action == SessionBaseModel::PowerAction::RequireShutdown || action == SessionBaseModel::PowerAction::RequireRestart || action == SessionBaseModel::PowerAction::RequireLogout)) {
        m_confirm = false;

        InhibitWarnView *view = new InhibitWarnView(action, this);
        view->setFocusPolicy(Qt::NoFocus);
        setFocusPolicy(Qt::NoFocus);
        view->setAction(action);
        if (action == SessionBaseModel::PowerAction::RequireShutdown) {
            view->setAcceptReason(tr("Shut down"));
            view->setInhibitConfirmMessage(tr("Are you sure you want to shut down?"));
        } else if (action == SessionBaseModel::PowerAction::RequireRestart) {
            view->setAcceptReason(tr("Reboot"));
            view->setInhibitConfirmMessage(tr("Are you sure you want to reboot?"));
        } else if (action == SessionBaseModel::PowerAction::RequireLogout) {
            view->setAcceptReason(tr("Log out"));
            view->setInhibitConfirmMessage(tr("Are you sure you want to log out?"));
        }

        m_warningView = view;

        m_mainLayout->addWidget(m_warningView);
        m_mainLayout->setAlignment(m_warningView, Qt::AlignCenter);
        m_mainLayout->setCurrentWidget(m_warningView);

        connect(view, &InhibitWarnView::cancelled, this, &ShutdownWidget::onCancelAction);
        connect(view, &InhibitWarnView::actionInvoked, [this, action] {
            m_model->setPowerAction(action);
        });

        return false;
    }

    m_model->setPowerAction(action);

    return true;
}

void ShutdownWidget::enterKeyPushed()
{
    if (m_warningView) {
        m_warningView->buttonClickHandle();
        return;
    }

    if (m_systemMonitor && m_systemMonitor->state() == SystemMonitor::Enter) {
        runSystemMonitor();
        return;
    }

    if (m_currentSelectedBtn->isDisabled())
        return;

    if (m_currentSelectedBtn == m_requireShutdownButton)
        beforeInvokeAction(SessionBaseModel::PowerAction::RequireShutdown);
    else if (m_currentSelectedBtn == m_requireRestartButton)
        beforeInvokeAction(SessionBaseModel::PowerAction::RequireRestart);
    else if (m_currentSelectedBtn == m_requireSuspendButton)
        beforeInvokeAction(SessionBaseModel::PowerAction::RequireSuspend);
    else if (m_currentSelectedBtn == m_requireHibernateButton)
        beforeInvokeAction(SessionBaseModel::PowerAction::RequireHibernate);
    else if (m_currentSelectedBtn == m_requireLockButton)
        beforeInvokeAction(SessionBaseModel::PowerAction::RequireLock);
    else if (m_currentSelectedBtn == m_requireSwitchUserBtn)
        beforeInvokeAction(SessionBaseModel::PowerAction::RequireSwitchUser);
    else if (m_currentSelectedBtn == m_requireSwitchSystemBtn)
        beforeInvokeAction(SessionBaseModel::PowerAction::RequireSwitchSystem);
    else if (m_currentSelectedBtn == m_requireLogoutButton)
        beforeInvokeAction(SessionBaseModel::PowerAction::RequireLogout);
}

void ShutdownWidget::enableHibernateBtn(bool enable)
{
    m_requireHibernateButton->setVisible(enable);
}

void ShutdownWidget::enableSleepBtn(bool enable)
{
    m_requireSuspendButton->setVisible(enable);
}

void ShutdownWidget::initUI()
{
    setFocusPolicy(Qt::StrongFocus);
    m_requireShutdownButton = new RoundItemButton(this);
    m_requireShutdownButton->setFocusPolicy(Qt::NoFocus);
    m_requireShutdownButton->setObjectName("RequireShutDownButton");
    m_requireShutdownButton->setAutoExclusive(true);
    updateTr(m_requireShutdownButton, "Shut down");

    m_requireRestartButton = new RoundItemButton(tr("Reboot"), this);
    m_requireRestartButton->setFocusPolicy(Qt::NoFocus);
    m_requireRestartButton->setObjectName("RequireRestartButton");
    m_requireRestartButton->setAutoExclusive(true);
    updateTr(m_requireRestartButton, "Reboot");

    m_requireSuspendButton = new RoundItemButton(tr("Suspend"), this);
    m_requireSuspendButton->setFocusPolicy(Qt::NoFocus);
    m_requireSuspendButton->setObjectName("RequireSuspendButton");
    m_requireSuspendButton->setAutoExclusive(true);
    updateTr(m_requireSuspendButton, "Suspend");

    m_requireHibernateButton = new RoundItemButton(tr("Hibernate"), this);
    m_requireHibernateButton->setFocusPolicy(Qt::NoFocus);
    m_requireHibernateButton->setObjectName("RequireHibernateButton");
    m_requireHibernateButton->setAutoExclusive(true);
    updateTr(m_requireHibernateButton, "Hibernate");

    m_requireLockButton = new RoundItemButton(tr("Lock"));
    m_requireLockButton->setFocusPolicy(Qt::NoFocus);
    m_requireLockButton->setObjectName("RequireLockButton");
    m_requireLockButton->setAutoExclusive(true);
    updateTr(m_requireLockButton, "Lock");

    m_requireLogoutButton = new RoundItemButton(tr("Log out"));
    m_requireLogoutButton->setFocusPolicy(Qt::NoFocus);
    m_requireLogoutButton->setObjectName("RequireLogoutButton");
    m_requireLogoutButton->setAutoExclusive(true);
    updateTr(m_requireLogoutButton, "Log out");

    m_requireSwitchUserBtn = new RoundItemButton(tr("Switch user"));
    m_requireSwitchUserBtn->setFocusPolicy(Qt::NoFocus);
    m_requireSwitchUserBtn->setObjectName("RequireSwitchUserButton");
    m_requireSwitchUserBtn->setAutoExclusive(true);
    updateTr(m_requireSwitchUserBtn, "Switch user");
    m_requireSwitchUserBtn->setVisible(false);

    if (m_switchosInterface->isDualOsSwitchAvail()) {
        m_requireSwitchSystemBtn = new RoundItemButton(tr("Switch system"));
        m_requireSwitchSystemBtn->setFocusPolicy(Qt::NoFocus);
        m_requireSwitchSystemBtn->setObjectName("RequireSwitchSystemButton");
        m_requireSwitchSystemBtn->setAutoExclusive(true);
        updateTr(m_requireSwitchSystemBtn, "Switch system");
    }

    m_btnList.append(m_requireShutdownButton);
    m_btnList.append(m_requireRestartButton);
    m_btnList.append(m_requireSuspendButton);
    m_btnList.append(m_requireHibernateButton);
    m_btnList.append(m_requireLockButton);
    m_btnList.append(m_requireSwitchUserBtn);
    if(m_requireSwitchSystemBtn) {
        m_btnList.append(m_requireSwitchSystemBtn);
    }
    m_btnList.append(m_requireLogoutButton);

    m_shutdownLayout = new QHBoxLayout;
    m_shutdownLayout->setMargin(0);
    m_shutdownLayout->setSpacing(10);
    m_shutdownLayout->addStretch();
    m_shutdownLayout->addWidget(m_requireShutdownButton);
    m_shutdownLayout->addWidget(m_requireRestartButton);
    m_shutdownLayout->addWidget(m_requireSuspendButton);
    m_shutdownLayout->addWidget(m_requireHibernateButton);
    m_shutdownLayout->addWidget(m_requireLockButton);
    m_shutdownLayout->addWidget(m_requireSwitchUserBtn);
    if(m_requireSwitchSystemBtn) {
        m_shutdownLayout->addWidget(m_requireSwitchSystemBtn);
    }
    m_shutdownLayout->addWidget(m_requireLogoutButton);
    m_shutdownLayout->addStretch(0);

    m_shutdownFrame = new QFrame;
    m_shutdownFrame->setLayout(m_shutdownLayout);

    m_actionLayout = new QVBoxLayout;
    m_actionLayout->setMargin(0);
    m_actionLayout->setSpacing(10);
    m_actionLayout->addStretch();
    m_actionLayout->addWidget(m_shutdownFrame);
    m_actionLayout->setAlignment(m_shutdownFrame, Qt::AlignCenter);
    m_actionLayout->addStretch();

    if (findValueByQSettings<bool>(DDESESSIONCC::session_ui_configs, "Shutdown", "enableSystemMonitor", true)) {
        QFile file("/usr/bin/deepin-system-monitor");
        if (file.exists()) {
            m_systemMonitor = new SystemMonitor;
            m_systemMonitor->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
            m_systemMonitor->setFocusPolicy(Qt::NoFocus);
            setFocusPolicy(Qt::StrongFocus);
            m_actionLayout->addWidget(m_systemMonitor);
            m_actionLayout->setAlignment(m_systemMonitor, Qt::AlignHCenter);
        }
    }

    m_actionFrame = new QFrame;
    m_actionFrame->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_actionFrame->setFocusPolicy(Qt::NoFocus);
    m_actionFrame->setLayout(m_actionLayout);

    m_mainLayout = new QStackedLayout;
    m_mainLayout->setMargin(0);
    m_mainLayout->setSpacing(0);
    m_mainLayout->addWidget(m_actionFrame);
    m_mainLayout->setAlignment(m_actionFrame, Qt::AlignCenter);
    setLayout(m_mainLayout);

    updateStyle(":/skin/requireshutdown.qss", this);

    // refresh language
    for (auto it = m_trList.constBegin(); it != m_trList.constEnd(); ++it) {
        it->first(qApp->translate("ShutdownWidget", it->second.toUtf8()));
    }
}

void ShutdownWidget::leftKeySwitch()
{
    m_btnList.at(m_index)->updateState(RoundItemButton::Normal);
    if (m_index <= 0) {
        m_index = m_btnList.length() - 1;
    } else {
        m_index  -= 1;
    }

    for (int i = m_btnList.size(); i != 0; --i) {
        int index = (m_index + i) % m_btnList.size();

        if (m_btnList[index]->isVisible()) {
            m_index = index;
            break;
        }
    }

    m_currentSelectedBtn = m_btnList.at(m_index);
    m_currentSelectedBtn->updateState(RoundItemButton::Checked);

    m_frameDataBind->updateValue("ShutdownWidget", m_index);
}

void ShutdownWidget::rightKeySwitch()
{
    m_btnList.at(m_index)->updateState(RoundItemButton::Normal);

    if (m_index == m_btnList.size() - 1) {
        m_index = 0;
    } else {
        ++m_index;
    }

    for (int i = 0; i < m_btnList.size(); ++i) {
        int index = (m_index + i) % m_btnList.size();

        if (m_btnList[index]->isVisible()) {
            m_index = index;
            break;
        }
    }

    m_currentSelectedBtn = m_btnList.at(m_index);
    m_currentSelectedBtn->updateState(RoundItemButton::Checked);

    m_frameDataBind->updateValue("ShutdownWidget", m_index);
}

QList<InhibitWarnView::InhibitorData> ShutdownWidget::listInhibitors(const SessionBaseModel::PowerAction action)
{
    QList<InhibitWarnView::InhibitorData> inhibitorList;

    if (m_login1Inter->isValid()) {
        qDebug() <<  "m_login1Inter is valid!";

        QDBusPendingReply<InhibitorsList> reply = m_login1Inter->ListInhibitors();
        reply.waitForFinished();

        if (!reply.isError()) {
            InhibitorsList inhibitList = qdbus_cast<InhibitorsList>(reply.argumentAt(0));
            qDebug() << "inhibitList:" << inhibitList.count();

            QString type;

            switch (action) {
            case SessionBaseModel::PowerAction::RequireShutdown:
            case SessionBaseModel::PowerAction::RequireRestart:
            case SessionBaseModel::PowerAction::RequireSwitchSystem:
            case SessionBaseModel::PowerAction::RequireLogout:
                type = "shutdown";
                break;
            case SessionBaseModel::PowerAction::RequireSuspend:
            case SessionBaseModel::PowerAction::RequireHibernate:
                type = "sleep";
                break;
            default:
                return {};
            }

            for (int i = 0; i < inhibitList.count(); i++) {
                // Just take care of DStore's inhibition, ignore others'.
                const Inhibit &inhibitor = inhibitList.at(i);
                if (inhibitor.what.split(':', QString::SkipEmptyParts).contains(type)
                        && !m_inhibitorBlacklists.contains(inhibitor.who)) {

                    // 待机时，非block暂不处理，因为目前没有倒计时待机功能
                    if (type == "sleep" && inhibitor.mode != "block")
                        continue;

                    InhibitWarnView::InhibitorData inhibitData;
                    inhibitData.who = inhibitor.who;
                    inhibitData.why = inhibitor.why;
                    inhibitData.mode = inhibitor.mode;
                    inhibitData.pid = inhibitor.pid;

                    // 读取翻译后的文本，读取应用图标
                    QDBusConnection connection = QDBusConnection::sessionBus();
                    if (!inhibitor.uid) {
                        connection = QDBusConnection::systemBus();
                    }

                    if (connection.interface()->isServiceRegistered(inhibitor.who)) {

                        QDBusInterface ifc(inhibitor.who, "/com/deepin/InhibitHint", "com.deepin.InhibitHint", connection);
                        QDBusMessage msg = ifc.call("Get", getenv("LANG"), inhibitor.why);
                        if (msg.type() == QDBusMessage::ReplyMessage) {
                            InhibitHint inhibitHint = qdbus_cast<InhibitHint>(msg.arguments().at(0).value<QDBusArgument>());

                            if (!inhibitHint.why.isEmpty()) {
                                inhibitData.who = inhibitHint.name;
                                inhibitData.why = inhibitHint.why;
                                inhibitData.icon = inhibitHint.icon;
                            }
                        }
                    }

                    inhibitorList.append(inhibitData);
                }
            }

            qDebug() << "List of valid '" << type << "' inhibitors:";

            for (const InhibitWarnView::InhibitorData &data : inhibitorList) {
                qDebug() << "who:" << data.who;
                qDebug() << "why:" << data.why;
                qDebug() << "pid:" << data.pid;
            }

            qDebug() << "End list inhibitor";
        } else {
            qWarning() << "D-Bus request reply error:" << reply.error().message();
        }
    } else {
        qWarning() << "shutdown login1Manager error!";
    }

    return inhibitorList;
}

void ShutdownWidget::onStatusChanged(SessionBaseModel::ModeStatus status)
{
    //根据当前是锁屏还是关机,设置按钮可见状态,同时需要判官切换用户按钮是否允许可见
    RoundItemButton * roundItemButton = m_requireShutdownButton;
    if (m_model->currentModeState() == SessionBaseModel::ModeStatus::ShutDownMode) {
        m_requireLockButton->setVisible(true);
        m_requireSwitchUserBtn->setVisible(m_switchUserEnable);
        if (m_requireSwitchSystemBtn) {
            m_requireSwitchSystemBtn->setVisible(true);
        }
        m_requireLogoutButton->setVisible(true);
        roundItemButton = m_requireLockButton;
    } else {
        m_requireLockButton->setVisible(false);
        m_requireSwitchUserBtn->setVisible(false);
        if (m_requireSwitchSystemBtn) {
            m_requireSwitchSystemBtn->setVisible(false);
        }
        m_requireLogoutButton->setVisible(false);
        roundItemButton = m_requireShutdownButton;
    }

    int index = m_btnList.indexOf(roundItemButton);
    roundItemButton->updateState(RoundItemButton::Checked);
    m_frameDataBind->updateValue("ShutdownWidget", index);

    if (m_systemMonitor) {
        m_systemMonitor->setVisible(status == SessionBaseModel::ModeStatus::ShutDownMode);
    }
}

void ShutdownWidget::runSystemMonitor()
{
    QProcess::startDetached("/usr/bin/deepin-system-monitor");

    if (m_systemMonitor) {
        m_systemMonitor->clearFocus();
        m_systemMonitor->setState(SystemMonitor::Leave);
    }

    hideToplevelWindow();
}

void ShutdownWidget::onCancelAction()
{
    emit abortOperation();
    hideToplevelWindow();
}

void ShutdownWidget::recoveryLayout()
{
    //关机或重启确认前会隐藏所有按钮,取消重启或关机后隐藏界面时重置按钮可见状态
    //同时需要判断切换用户按钮是否允许可见
    m_requireShutdownButton->setVisible(true);
    m_requireRestartButton->setVisible(true);
    enableHibernateBtn(m_model->canSleep());
    enableSleepBtn(m_model->hasSwap());
    m_requireLockButton->setVisible(true);
    m_requireSwitchUserBtn->setVisible(m_switchUserEnable);

    if (m_systemMonitor) {
        m_systemMonitor->setVisible(false);
    }

    if (m_warningView != nullptr) {
        m_mainLayout->removeWidget(m_warningView);
        m_warningView->deleteLater();
        m_warningView = nullptr;
    }

    m_mainLayout->setCurrentWidget(m_actionFrame);
    setFocusPolicy(Qt::StrongFocus);
}

void ShutdownWidget::onRequirePowerAction(SessionBaseModel::PowerAction powerAction, bool requireConfirm)
{
    m_confirm = requireConfirm;

    if (m_model->currentModeState() == SessionBaseModel::ModeStatus::ShutDownMode) {
        switch (powerAction) {
        case SessionBaseModel::PowerAction::RequireShutdown:
        case SessionBaseModel::PowerAction::RequireRestart:
        case SessionBaseModel::PowerAction::RequireSwitchSystem:
        case SessionBaseModel::PowerAction::RequireLogout:
            beforeInvokeAction(powerAction);
            break;
        default:
            m_model->setPowerAction(powerAction);
            break;
        }
    } else {
        m_model->setPowerAction(powerAction);
        emit abortOperation();
    }
}

void ShutdownWidget::setUserSwitchEnable(bool enable)
{
    //接收到用户列表变更信号号,记录切换用户是否允许可见,再根据当前是锁屏还是关机设置切换按钮可见状态
    if (m_switchUserEnable == enable) {
        return;
    }

    m_switchUserEnable = enable;
    m_requireSwitchUserBtn->setVisible(m_switchUserEnable && m_model->currentModeState() == SessionBaseModel::ModeStatus::ShutDownMode);
}

void ShutdownWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Left:
        leftKeySwitch();
        break;
    case Qt::Key_Right:
        rightKeySwitch();
        break;
    case Qt::Key_Enter:
    case Qt::Key_Return:
        enterKeyPushed();
        break;
    case Qt::Key_Tab: {
        if (m_systemMonitor && m_currentSelectedBtn->isVisible() && m_systemMonitor->isVisible()) {
            if (m_currentSelectedBtn && m_currentSelectedBtn->isChecked()) {
                m_currentSelectedBtn->setChecked(false);
                m_systemMonitor->setState(SystemMonitor::Enter);
            } else if (m_systemMonitor->state() == SystemMonitor::Enter) {
                m_systemMonitor->setState(SystemMonitor::Leave);
                m_currentSelectedBtn->setChecked(true);
            }
        }
        break;
    }
    default:
        break;
    }

    QFrame::keyReleaseEvent(event);
}

bool ShutdownWidget::event(QEvent *e)
{
    if (e->type() == QEvent::LanguageChange) {
        // refresh language
        for (auto it = m_trList.constBegin(); it != m_trList.constEnd(); ++it) {
            it->first(qApp->translate("ShutdownWidget", it->second.toUtf8()));
        }
    }

    if (e->type() == QEvent::FocusIn) {
        if (m_index < 0) {
            m_index = 0;
        }
        m_frameDataBind->updateValue("ShutdownWidget", m_index);
    }

    return QFrame::event(e);
}

ShutdownWidget::~ShutdownWidget()
{
}

void ShutdownWidget::setModel(SessionBaseModel *const model)
{
    m_model = model;

    connect(model, &SessionBaseModel::onRequirePowerAction, this, &ShutdownWidget::onRequirePowerAction);

    connect(model, &SessionBaseModel::onHasSwapChanged, this, &ShutdownWidget::enableHibernateBtn);
    enableHibernateBtn(model->hasSwap());

    connect(model, &SessionBaseModel::canSleepChanged, this, &ShutdownWidget::enableSleepBtn);
    enableSleepBtn(model->canSleep());
}
