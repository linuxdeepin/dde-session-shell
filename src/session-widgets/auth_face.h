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

#ifndef AUTHFACE_H
#define AUTHFACE_H

#include "auth_module.h"

#define Face_Auth QStringLiteral(":/misc/images/auth/face.svg")

class AuthFace : public AuthModule
{
    Q_OBJECT
public:
    explicit AuthFace(QWidget *parent = nullptr);

    void reset();
public slots:
    void setAuthState(const int state, const QString &result) override;
    void setAnimationState(const bool start) override;
    void setLimitsInfo(const LimitsInfo &info) override;
    void setAuthFactorType(AuthFactorType authFactorType) override;

protected:
    virtual bool eventFilter(QObject *watched, QEvent *event);

private:
    void initUI();
    void initConnections();
    void updateUnlockPrompt() override;
    void doAnimation() override;

signals:
    void retryButtonVisibleChanged(bool visible);

private:
    int m_aniIndex;
    DLabel *m_textLabel;
};

#endif // AUTHFACE_H
