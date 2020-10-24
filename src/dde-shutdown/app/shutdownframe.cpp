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

#include <QDebug>

#include "shutdownframe.h"
#include "src/session-widgets/sessionbasemodel.h"
#include "src/dde-shutdown/dbusshutdownagent.h"

const QString WallpaperKey = "pictureUri";

ShutdownFrame::ShutdownFrame(SessionBaseModel *const model, QWidget *parent)
    : FullscreenBackground(parent)
    , m_model(model)
{
    QTimer::singleShot(0, this, [ = ] {
        auto user = model->currentUser();
        if (user != nullptr) updateBackground(QPixmap(user->desktopBackgroundPath()));
    });

    m_shutdownFrame = new ContentWidget(this);
    m_shutdownFrame->setModel(model);
    setContent(m_shutdownFrame);
    m_shutdownFrame->hide();

    connect(m_shutdownFrame, &ContentWidget::requestBackground,
            this, static_cast<void (ShutdownFrame::*)(const QString &)>(&ShutdownFrame::updateBackground));

    connect(m_shutdownFrame, &ContentWidget::buttonClicked, this, &ShutdownFrame::buttonClicked);

    connect(model, &SessionBaseModel::visibleChanged, this, [ = ](bool visible) {
        if (visible) {
            // refresh hibernate and sleep function.
            emit model->onStatusChanged(SessionBaseModel::PowerMode);
        }
    });

    m_shutdownFrame->initBackground();
}

bool ShutdownFrame::powerAction(const Actions action)
{
    return m_shutdownFrame->powerAction(action);
}

void ShutdownFrame::setConfirm(const bool confirm)
{
    m_shutdownFrame->setConfirm(confirm);
}

void ShutdownFrame::showEvent(QShowEvent *event)
{
    Q_EMIT requestEnableHotzone(false);

    m_model->setIsShow(true);

    return FullscreenBackground::showEvent(event);
}

void ShutdownFrame::hideEvent(QHideEvent *event)
{
    Q_EMIT requestEnableHotzone(true);

    m_model->setIsShow(false);

    m_shutdownFrame->recoveryLayout();

    return FullscreenBackground::hideEvent(event);
}

bool ShutdownFrame::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::KeyRelease) {
        QString  keyValue = "";
        switch (static_cast<QKeyEvent *>(event)->key()) {
        // 需要捕捉poweroff键进行处理
        case Qt::Key_PowerOff: {
            if (!handlePoweroffKey()) {
                keyValue = "power-off";
            }
            break;
        }
        }
        if (keyValue != "") {
            emit sendKeyValue(keyValue);
        }
    }
    return QObject::eventFilter(watched, event);
}

/**
 * @brief ShutdownFrame::handlePoweroffKey
 * 根据Power键的行为对捕捉到Power键后做特殊处理
 * @return
 */
bool ShutdownFrame::handlePoweroffKey()
{
    QDBusInterface powerInter("com.deepin.daemon.Power","/com/deepin/daemon/Power","com.deepin.daemon.Power");
    if (!powerInter.isValid()) {
        return false;
    }
    int action = powerInter.property("LinePowerPressPowerBtnAction").toInt();
    // 需要特殊处理：无任何操作(4)
    if (action == 4) {
       return true;
    }
    return false;
}

ShutdownFrame::~ShutdownFrame()
{
}

ShutdownFrontDBus::ShutdownFrontDBus(DBusShutdownAgent *parent,SessionBaseModel* model)
    :QDBusAbstractAdaptor(parent)
    ,m_parent(parent)
    ,m_model(model)
{
}

ShutdownFrontDBus::~ShutdownFrontDBus()
{
}

void ShutdownFrontDBus::Shutdown()
{
    m_parent->Shutdown();
}

void ShutdownFrontDBus::Restart()
{
    m_parent->Restart();
}

void ShutdownFrontDBus::Logout()
{
    m_parent->Logout();
}

void ShutdownFrontDBus::Suspend()
{
    m_parent->Suspend();
}

void ShutdownFrontDBus::Hibernate()
{
    m_parent->Hibernate();
}

void ShutdownFrontDBus::SwitchUser()
{
    m_parent->SwitchUser();
}

void ShutdownFrontDBus::Show()
{
    if ( m_model != nullptr && !m_model->isLocked() )
        m_parent->Show();
}
