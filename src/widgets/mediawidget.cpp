/*
 * Copyright (C) 2011 ~ 2018 Deepin Technology Co., Ltd.
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

#include "mediawidget.h"
#include "src/global_util/util_updateui.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWheelEvent>

MediaWidget::MediaWidget(QWidget *parent) : QWidget(parent)
{
    initUI();
    initConnect();
}

void MediaWidget::initUI()
{
    m_dmprisWidget = new DMPRISControl;
    m_dmprisWidget->setFixedWidth(200);
    m_dmprisWidget->setPictureVisible(false);

    QHBoxLayout *mainlayout = new QHBoxLayout;

    mainlayout->addWidget(m_dmprisWidget, 0, Qt::AlignBottom);

    setLayout(mainlayout);

    //updateStyle(":/skin/mediawidget.qss", this);
}

void MediaWidget::initConnect()
{
    connect(m_dmprisWidget, &DMPRISControl::mprisAcquired, this, &MediaWidget::changeVisible);
    connect(m_dmprisWidget, &DMPRISControl::mprisLosted, this, &MediaWidget::changeVisible);

    changeVisible();
}

void MediaWidget::initMediaPlayer()
{
    QDBusInterface dbusInter("org.freedesktop.DBus", "/", "org.freedesktop.DBus", QDBusConnection::sessionBus(), this);

    QDBusReply<QStringList> response = dbusInter.call("ListNames");
    const QStringList &serviceList = response.value();
    QString service = QString();
    for (const QString &serv : serviceList) {
        if (!serv.startsWith("org.mpris.MediaPlayer2.")) {
            continue;
        }
        service = serv;
        break;
    }
    if (service.isEmpty()) {
        qDebug() << "media player dbus has not started, waiting for signal...";
        QDBusConnectionInterface *dbusDaemonInterface = QDBusConnection::sessionBus().interface();
        connect(dbusDaemonInterface, &QDBusConnectionInterface::serviceOwnerChanged, this,
        [ = ](const QString & name, const QString & oldOwner, const QString & newOwner) {
            Q_UNUSED(oldOwner);
            if (name.startsWith("org.mpris.MediaPlayer2.") && !newOwner.isEmpty()) {
                initMediaPlayer();
                disconnect(dbusDaemonInterface);
            }
        }
               );
        return;
    }

    qDebug() << "got media player dbus service: " << service;

    m_dbusInter = new DBusMediaPlayer2(service, "/org/mpris/MediaPlayer2", QDBusConnection::sessionBus(), this);

    m_dbusInter->MetadataChanged();
    m_dbusInter->PlaybackStatusChanged();
    m_dbusInter->VolumeChanged();
}

void MediaWidget::changeVisible()
{
    const bool isWorking = m_dmprisWidget->isWorking();
    m_dmprisWidget->setVisible(isWorking);
}
