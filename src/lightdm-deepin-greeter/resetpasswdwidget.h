/*
* Copyright (C) 2021 ~ 2021 Uniontech Software Technology Co.,Ltd.
*
* Author:     yinjie <yinjie@uniontech.com>
*
* Maintainer: yinjie <yinjie@uniontech.com>
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

#ifndef RESETPASSWDWIDGET_H
#define RESETPASSWDWIDGET_H

#include "auth_widget.h"

#include <QWidget>

class DLineEditEx;
class QVBoxLayout;

class ResetPasswdWidget : public AuthWidget
{
    Q_OBJECT
public:
    explicit ResetPasswdWidget(QWidget *parent = nullptr);

    void setPrompt(const QString &prompt);
    void setMessage(const QString &message);

    virtual void setModel(const SessionBaseModel *model) override;

private:
    void initUI();
    void initConnections();

signals:
    void respondPasswd(const QString& );

public slots:

private:
    QVBoxLayout *m_mainLayout;
    DLineEditEx *m_passwdEdit;
};

#endif // RESETPASSWDWIDGET_H
