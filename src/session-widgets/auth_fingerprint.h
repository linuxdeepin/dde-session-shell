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

#ifndef AUTHFINGERPRINT_H
#define AUTHFINGERPRINT_H

#include "auth_module.h"

#define Fingerprint_Auth QStringLiteral(":/misc/images/auth/fingerprint.svg")

class AuthFingerprint : public AuthModule
{
    Q_OBJECT
public:
    explicit AuthFingerprint(QWidget *parent = nullptr);

    void reset();

    void setAnimationState(const bool start) override;
    void setAuthState(const int state, const QString &result) override;
    void setLimitsInfo(const LimitsInfo &info) override;
    void setAuthFactorType(AuthFactorType authFactorType) override;
    void doAnimation() override;

protected:
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void initUI();
    void initConnections();
    void updateUnlockPrompt() override;

private:
    int m_aniIndex;
    DLabel *m_textLabel;
};

#endif // AUTHFINGERPRINT_H
