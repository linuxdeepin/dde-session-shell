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

#include "dbusshutdownfrontservice.h"

DBusShutdownFrontService::DBusShutdownFrontService(DBusShutdownAgent *parent)
    : QDBusAbstractAdaptor(parent)
{
    setAutoRelaySignals(true);
}

DBusShutdownFrontService::~DBusShutdownFrontService()
{

}

void DBusShutdownFrontService::Show()
{
    parent()->show();
}

void DBusShutdownFrontService::Shutdown()
{
    parent()->Shutdown();
}

void DBusShutdownFrontService::Restart()
{
    parent()->Restart();
}

void DBusShutdownFrontService::Logout()
{
    parent()->Logout();
}

void DBusShutdownFrontService::Suspend()
{
    parent()->Suspend();
}

void DBusShutdownFrontService::Hibernate()
{
    parent()->Hibernate();
}

void DBusShutdownFrontService::SwitchUser()
{
    parent()->SwitchUser();
}

void DBusShutdownFrontService::Lock()
{
    parent()->Lock();
}
