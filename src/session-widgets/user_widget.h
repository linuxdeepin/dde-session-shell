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

#ifndef USERWIDGET_H
#define USERWIDGET_H

#include "userinfo.h"

#include <DBlurEffectWidget>
#include <DLabel>

#include <QWidget>

DWIDGET_USE_NAMESPACE

const int UserFrameWidth = 226;
const int UserFrameHeight = 167;

class UserAvatar;
class QVBoxLayout;
class UserWidget : public QWidget
{
    Q_OBJECT
public:
    explicit UserWidget(QWidget *parent = nullptr);

    void setUser(std::shared_ptr<User> user);

    inline bool isSelected() const { return m_isSelected; }
    void setSelected(bool isSelected);
    void setFastSelected(bool isSelected);

    inline uint uid() const { return m_uid; }
    void setUid(const uint uid);

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void initUI();
    void initConnections();

    void setAvatar(const QString &avatar);
    void setName(const QString &name);
    void setNameFont(const QFont &font);
    void setLoginStatus(const bool isLogin);

    void updateBlurEffectGeometry();

private:
    bool m_isSelected;
    uint m_uid;

    QVBoxLayout *m_mainLayout; // 登录界面布局

    DBlurEffectWidget *m_blurEffectWidget; // 模糊背景
    UserAvatar *m_avatar;                  // 用户头像

    DLabel *m_loginStatus; // 用户登录状态
    DLabel *m_nameLabel;   // 用户名
    QWidget *m_nameWidget; // 用户名控件

    std::shared_ptr<User> m_user;
};

#endif // USERWIDGET_H
