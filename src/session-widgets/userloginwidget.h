/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     lixin <lixin_cm@deepin.com>
 *
 * Maintainer: lixin <lixin_cm@deepin.com>
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

#ifndef USERLOGINWIDGET_H
#define USERLOGINWIDGET_H

#include "sessionbasemodel.h"

#include <DBlurEffectWidget>
#include <DPasswdEditAnimated>

DWIDGET_USE_NAMESPACE

class QLabel;
class QPushButton;
class User;
class UserAvatar;
class LoginButton;
class LockPasswordWidget;
class FrameDataBind;
class OtherUserInput;
class QVBoxLayout;

class UserLoginWidget : public QWidget
{
    Q_OBJECT
public:
    enum WidgetShowType {
        NoPasswordType,
        NormalType,
        IDAndPasswordType
    };

    explicit UserLoginWidget(QWidget *parent = nullptr);
    void resetAllState();
    void grabKeyboard();
    void setFaildTipMessage(const QString &message);
    void setWidgetShowType(WidgetShowType showType);
    void setName(const QString &name);
    void setAvatar(const QString &avatar);
    void setUserAvatarSize(int width, int height);

signals:
    void requestAuthUser(const QString &password);

public slots:
    void disablePassword(bool disable);
    void updateAuthType(SessionBaseModel::AuthType type);
    void refreshBlurEffectPosition();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    void initUI();
    void initConnect();
    void updateUI();

private:
    DBlurEffectWidget *m_blurEffectWidget;                  //阴影窗体
    UserAvatar *m_userAvatar;                               //用户头像
    QLabel *m_nameLbl;                                      //用户名
    DPasswdEditAnimated *m_passwordEdit;                    //密码输入框
    QMap<uint, QString> m_passwords;
    QList<QMetaObject::Connection> m_currentUserConnects;
    SessionBaseModel::AuthType m_authType;                  //认证类型
    QPushButton *m_loginButton;                             //test code
    OtherUserInput *m_otherUserInput;
    WidgetShowType m_showType;                              //窗体显示模式,分为无密码登录模式\正常模式\ID和密码登录模式
    QVBoxLayout *m_userLayout;                              //用户输入框布局
    QVBoxLayout *m_lockLayout;                              //解锁按钮布局
};

#endif // USERLOGINWIDGET_H
