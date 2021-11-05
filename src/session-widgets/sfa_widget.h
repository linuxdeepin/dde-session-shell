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

#ifndef SFAWIDGET_H
#define SFAWIDGET_H

#include "auth_widget.h"
#include "userinfo.h"

#include <DArrowRectangle>
#include <DBlurEffectWidget>
#include <DButtonBox>
#include <DClipEffectWidget>
#include <DFloatingButton>
#include <DLabel>

#include <QWidget>

class AuthModule;
class KbLayoutWidget;
class KeyboardMonitor;

class DLineEditEx;
class QVBoxLayout;
class UserAvatar;

class SFAWidget : public AuthWidget
{
    Q_OBJECT

public:
    explicit SFAWidget(QWidget *parent = nullptr);

    void setModel(const SessionBaseModel *model) override;
    void setAuthType(const int type) override;
    void setAuthStatus(const int type, const int status, const QString &message) override;
    void syncResetPasswordUI();

public slots:
    void onRetryButtonVisibleChanged(bool visible);

protected:
    void resizeEvent(QResizeEvent *event);

private:
    void initUI();
    void initConnections();

    void initSingleAuth();
    void initPasswdAuth();
    void initFingerprintAuth();
    void initUKeyAuth();
    void initFaceAuth();
    void initIrisAuth();

    void checkAuthResult(const int type, const int status) override;

    void syncAuthType(const QVariant &value);
    void replaceWidget(AuthModule *authModule);

private:
    QVBoxLayout *m_mainLayout;

    DButtonBox *m_chooesAuthButtonBox; // 认证选择按钮
    DLabel *m_biometricAuthStatus;     // 生物认证状态
    AuthModule *m_currentAuth;            // 当前选中的认证，默认 single 用于兼容开源 PAM
    AuthModule *m_lastAuth;            // 上次成功的认证

    QMap<int, DButtonBoxButton *> m_authButtons;
    DFloatingButton *m_retryButton;
};

#endif // SFAWIDGET_H
