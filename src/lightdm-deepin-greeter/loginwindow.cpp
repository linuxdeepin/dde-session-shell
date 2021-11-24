/*
 * Copyright (C) 2011 ~ 2018 Deepin Technology Co., Ltd.
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

#include "loginwindow.h"
#include "logincontent.h"
#include "userinfo.h"
#include "resetpasswdwidget.h"

#include <QWindow>

LoginWindow::LoginWindow(SessionBaseModel *const model, QWidget *parent)
    : FullscreenBackground(model, parent)
    , m_loginContent(new LoginContent(model, this))
    , m_model(model)
{
    setAccessibleName("LoginWindow");
    updateBackground(m_model->currentUser()->greeterBackground());
    setContent(m_loginContent);
    m_loginContent->setVisible(false);

    connect(m_loginContent, &LockContent::requestBackground, this, [ = ](const QString & wallpaper) {
        updateBackground(wallpaper);
#ifdef DISABLE_LOGIN_ANI
        // 在认证成功以后会通过更改背景来实现登录动画，但是禁用登录动画的情况下，会立即调用startSession，
        // 导致当前进程被lightdm退掉，X上会残留上一帧的画面，可以看到输入框等画面。使用repaint()强制刷新背景来避免这个问题。
        repaint();
#endif
    });

    connect(model, &SessionBaseModel::authFinished, this, [ = ](bool successd) {
        setEnterEnable(!successd);
        if (successd) {
            m_loginContent->setVisible(!successd);
        }

#ifdef DISABLE_LOGIN_ANI
        // 在认证成功以后会通过更改背景来实现登录动画，但是禁用登录动画的情况下，会立即调用startSession，
        // 导致当前进程被lightdm退掉，X上会残留上一帧的画面，可以看到输入框等画面。使用repaint()强制刷新背景来避免这个问题。
        repaint();
#endif
    });

    connect(m_loginContent, &LockContent::requestSwitchToUser, this, &LoginWindow::requestSwitchToUser);
    connect(m_loginContent, &LockContent::requestSetLayout, this, &LoginWindow::requestSetKeyboardLayout);

    connect(m_loginContent, &LockContent::requestCheckAccount, this, &LoginWindow::requestCheckAccount);
    connect(m_loginContent, &LockContent::requestStartAuthentication, this, &LoginWindow::requestStartAuthentication);
    connect(m_loginContent, &LockContent::sendTokenToAuth, this, &LoginWindow::sendTokenToAuth);
    connect(m_loginContent, &LockContent::requestEndAuthentication, this, &LoginWindow::requestEndAuthentication);
    connect(m_loginContent, &LockContent::authFinished, this, [this]{
        setEnterEnable(true);
        if (m_model->currentUser()->expiredStatus() != User::ExpiredAlready)
            m_loginContent->hide();
        emit authFinished();
    });
    connect(m_loginContent, &LoginContent::respondPasswd, this, &LoginWindow::respondPasswd);
    connect(this, &LoginWindow::requestShowPrompt, m_loginContent, &LoginContent::showPrompt);
    connect(this, &LoginWindow::requestShowMessage, m_loginContent, &LoginContent::showMessage);
    connect(model, &SessionBaseModel::blackModeChanged, this, &FullscreenBackground::setIsBlackMode);
}

void LoginWindow::showEvent(QShowEvent *event)
{
    FullscreenBackground::showEvent(event);

    //greeter界面显示时，需要调用虚拟键盘
    m_model->setHasVirtualKB(true);
    m_model->setVisible(true);
}

void LoginWindow::hideEvent(QHideEvent *event)
{
    FullscreenBackground::hideEvent(event);

    //greeter界面隐藏时，需要结束虚拟键盘
    m_model->setHasVirtualKB(false);
    m_model->setVisible(false);
}
