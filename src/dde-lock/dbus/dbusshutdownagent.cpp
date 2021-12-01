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

#include "dbusshutdownagent.h"
#include "sessionbasemodel.h"

DBusShutdownAgent::DBusShutdownAgent(QObject *parent)
    : QObject(parent)
    , m_model(nullptr)
{

}

void DBusShutdownAgent::setModel(SessionBaseModel * const model)
{
    m_model = model;
}

void DBusShutdownAgent::show()
{
    if (!canShowShutDown()) {
        return;
    }

    m_model->setCurrentModeState(SessionBaseModel::ModeStatus::ShutDownMode);
    m_model->setVisible(true);
}

void DBusShutdownAgent::Shutdown()
{
    if (!canShowShutDown()) {
         //锁屏时允许关机
        emit m_model->onRequirePowerAction(SessionBaseModel::PowerAction::RequireShutdown, false);
    } else {
        m_model->setCurrentModeState(SessionBaseModel::ModeStatus::ShutDownMode);
        m_model->setVisible(true);
        emit m_model->onRequirePowerAction(SessionBaseModel::PowerAction::RequireShutdown, true);
    }
}

void DBusShutdownAgent::Restart()
{
    if (!canShowShutDown()) {
        //锁屏时允许重启
        emit m_model->onRequirePowerAction(SessionBaseModel::PowerAction::RequireRestart, false);
    } else {
        m_model->setCurrentModeState(SessionBaseModel::ModeStatus::ShutDownMode);
        m_model->setVisible(true);
        emit m_model->onRequirePowerAction(SessionBaseModel::PowerAction::RequireRestart, true);
    }
}

void DBusShutdownAgent::Logout()
{
    if (!canShowShutDown()) {
        return;
    }

    m_model->setCurrentModeState(SessionBaseModel::ModeStatus::ShutDownMode);
    m_model->setVisible(true);
    emit m_model->onRequirePowerAction(SessionBaseModel::PowerAction::RequireLogout, true);
}

void DBusShutdownAgent::Suspend()
{
    if (!canShowShutDown()) {
        //锁屏时允许待机
        m_model->setPowerAction(SessionBaseModel::RequireSuspend);
    } else {
        m_model->setCurrentModeState(SessionBaseModel::ModeStatus::ShutDownMode);
        m_model->setVisible(true);
        emit m_model->onRequirePowerAction(SessionBaseModel::RequireSuspend, true);
    }
}

void DBusShutdownAgent::Hibernate()
{
    if (!canShowShutDown()) {
        //锁屏时允许休眠
        m_model->setPowerAction(SessionBaseModel::RequireHibernate);
    } else {
        m_model->setCurrentModeState(SessionBaseModel::ModeStatus::ShutDownMode);
        m_model->setVisible(true);
        emit m_model->onRequirePowerAction(SessionBaseModel::RequireHibernate, true);
    }
}

void DBusShutdownAgent::SwitchUser()
{
    if (!canShowShutDown()) {
        return;
    }

    emit m_model->showUserList();
}

void DBusShutdownAgent::Lock()
{
    if (!canShowShutDown()) {
        return;
    }
    m_model->setCurrentModeState(SessionBaseModel::ModeStatus::ShutDownMode);
    m_model->setVisible(true);
    m_model->setPowerAction(SessionBaseModel::RequireLock);
}

bool DBusShutdownAgent::canShowShutDown()
{
    //如果当前界面已显示，而且不是关机模式，则当前已锁屏，因此不允许调用,以免在锁屏时被远程调用而进入桌面
    if (m_model->visible() && m_model->currentModeState() != SessionBaseModel::ModeStatus::ShutDownMode) {
        return false;
    }

    return true;
}
