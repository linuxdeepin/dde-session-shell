// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DBUSSHUTDOWNAGENT_H
#define DBUSSHUTDOWNAGENT_H

#include <QObject>

class SessionBaseModel;
class DBusShutdownAgent : public QObject
{
    Q_OBJECT
public:
    explicit DBusShutdownAgent(QObject *parent = nullptr);
    void setModel(SessionBaseModel *const model);
    void show();
    void Shutdown();
    void Restart();
    void Logout();
    void Suspend();
    void Hibernate();
    void SwitchUser();
    void Lock();
    void UpdateAndShutdown();
    void UpdateAndReboot();
    bool Visible() const;

private:
    bool canShowShutDown() const;
    bool isUpdating() const;

private:
    SessionBaseModel *m_model;
};

#endif // DBUSSHUTDOWNAGENT_H
