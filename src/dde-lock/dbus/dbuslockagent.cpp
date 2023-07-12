// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbuslockagent.h"
#include "sessionbasemodel.h"

DBusLockAgent::DBusLockAgent(QObject *parent) : QObject(parent), m_model(nullptr)
{

}

void DBusLockAgent::setModel(SessionBaseModel *const model)
{
    m_model = model;
}

void DBusLockAgent::Show()
{
    qInfo() << "DBusLockAgent::Show";
    if (isUpdating())
        return;

    m_model->setIsBlackMode(false);
    m_model->setIsHibernateModel(false);
    m_model->setVisible(true);
}

bool DBusLockAgent::Visible() const
{
    return m_model->visible();
}

void DBusLockAgent::ShowAuth(bool active)
{
    qInfo() << "DBusLockAgent::ShowAuth";
    if (isUpdating())
        return;

    Show();
    m_model->activeAuthChanged(active);
}

// 待机，enable=true：进入待机；enable=false：待机恢复
void DBusLockAgent::Suspend(bool enable)
{
    qInfo() << "DBusLockAgent::Suspend";
    if (isUpdating())
        return;

    if (enable) {
        m_model->setIsBlackMode(true);
        m_model->setVisible(true);
    } else {
        QDBusInterface infc("com.deepin.daemon.Power","/com/deepin/daemon/Power","com.deepin.daemon.Power");
        // 待机恢复需要密码
        bool bSuspendLock = infc.property("SleepLock").toBool();

        if (bSuspendLock) {
            m_model->setIsBlackMode(false);
            m_model->setVisible(true);
        } else {
            m_model->setVisible(false);
            emit m_model->visibleChanged(false);
        }
    }
}

void DBusLockAgent::Hibernate(bool enable)
{
    qInfo() << "DBusLockAgent::Hibernate";
    if (isUpdating())
        return;

    m_model->setIsHibernateModel(enable);
    m_model->setVisible(true);
}

void DBusLockAgent::ShowUserList()
{
    qInfo() << "DBusLockAgent::ShowUserList";
    if (isUpdating())
        return;

    emit m_model->showUserList();
}

/**
 * @brief 如果处于更新状态,那么不响应shutdown接口
 */
bool DBusLockAgent::isUpdating() const
{
    const bool updating = m_model->currentContentType() == SessionBaseModel::UpdateContent;
    qInfo() << "Is updating: " << updating;

    return updating;
}