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

#ifndef WARNINGCONTENT_H
#define WARNINGCONTENT_H

#include <QObject>
#include <QWidget>

#include "sessionbasewindow.h"
#include "sessionbasemodel.h"
#include "warningview.h"
#include "inhibitwarnview.h"
#include "multiuserswarningview.h"
#include "dbus/dbuslogin1manager.h"

class WarningContent : public SessionBaseWindow
{
    Q_OBJECT

public:
    explicit WarningContent(SessionBaseModel *const model, QWidget *parent = nullptr);
    ~WarningContent();
    void beforeInvokeAction(const SessionBaseModel::PowerAction action);   

protected:
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    QList<InhibitWarnView::InhibitorData> listInhibitors(const SessionBaseModel::PowerAction action);

private:
    SessionBaseModel *m_model;
    DBusLogin1Manager *m_login1Inter;
    WarningView * m_warningView = nullptr;
    QStringList m_inhibitorBlacklists;
};

class InhibitHint
{
public:
    QString name, icon, why;

    friend const QDBusArgument &operator>>(const QDBusArgument &argument, InhibitHint &obj)
    {
        argument.beginStructure();
        argument >> obj.name >> obj.icon >> obj.why;
        argument.endStructure();
        return argument;
    }
};

#endif // WARNINGCONTENT_H
