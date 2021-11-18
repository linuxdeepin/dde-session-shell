/*
* Copyright (C) 2021 ~ 2021 Uniontech Software Technology Co.,Ltd.
*
* Author:     Zhang Qipeng <zhangqipeng@uniontech.com>
*
* Maintainer: Zhang Qipeng <zhangqipeng@uniontech.com>
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

#ifndef MFAWIDGET_H
#define MFAWIDGET_H

#include "auth_widget.h"

#include <QWidget>

class MFAWidget : public AuthWidget
{
    Q_OBJECT

public:
    explicit MFAWidget(QWidget *parent = nullptr);

    void setModel(const SessionBaseModel *model) override;
    void setAuthType(const int type) override;
    void setAuthStatus(const int type, const int status, const QString &message) override;
    void autoUnlock();

private:
    void initUI();
    void initConnections();

    void initPasswdAuth();
    void initFingerprintAuth();
    void initUKeyAuth();
    void initFaceAuth();
    void initIrisAuth();

    void checkAuthResult(const int type, const int status) override;

    void updateFocusPosition();

private:
    int m_index;
    QVBoxLayout *m_mainLayout;
    QList<QWidget *> m_widgetList;
};

#endif // MFAWIDGET_H
