/*
 * Copyright (C) 2011 ~ 2020 Uniontech Technology Co., Ltd.
 *
 * Author:     chenjun <chenjun@uniontech>
 *
 * Maintainer: chenjun <chenjun@uniontech>
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

#ifndef DBUSSHUTDOWNFRONTSERVICE_H
#define DBUSSHUTDOWNFRONTSERVICE_H

#include "dbusshutdownagent.h"
#include <QDBusAbstractAdaptor>
#include <QtCore/QObject>
#include <QtDBus/QtDBus>

QT_BEGIN_NAMESPACE
class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QStringList;
class QVariant;
QT_END_NAMESPACE

/*
 * Proxy class for interface com.deepin.dde.shutdownFront
 */

class DBusShutdownFrontService : public QDBusAbstractAdaptor {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.dde.shutdownFront")

public:
    explicit DBusShutdownFrontService(DBusShutdownAgent *parent);
    virtual ~DBusShutdownFrontService();

    inline DBusShutdownAgent *parent() const
    { return static_cast<DBusShutdownAgent *>(QObject::parent()); }

public Q_SLOTS:
    void Show();
    void Shutdown();
    void Restart();
    void Logout();
    void Suspend();
    void Hibernate();
    void SwitchUser();
    void Lock();

Q_SIGNALS:
    void ChangKey(QString key);
};

#endif // DBUSSHUTDOWNFRONTSERVICE_H
