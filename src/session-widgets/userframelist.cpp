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

#include "userframelist.h"
#include "sessionbasemodel.h"
#include "userinfo.h"
#include "userloginwidget.h"

#include "dflowlayout.h"

#include <QVBoxLayout>
#include <QScrollArea>
#include <QMouseEvent>

const int UserFrameHeight = 150;
const int UserFrameWidth = 200;
const int UserFrameSpaceing = 30;
const int UserFrameRowCount = 2;
const int UserFrameColCount = 5;

UserFrameList::UserFrameList(QWidget *parent)
    : QWidget(parent)
    , m_scrollArea(new QScrollArea(this))
{
    initUI();
}

//设置SessionBaseModel，创建用户列表窗体
void UserFrameList::setModel(SessionBaseModel *model)
{
    m_model = model;

    connect(model, &SessionBaseModel::onUserAdded, this, &UserFrameList::addUser);
    connect(model, &SessionBaseModel::onUserRemoved, this, &UserFrameList::removeUser);

    QList<std::shared_ptr<User>> userList = m_model->userList();
    for (auto user : userList) {
        addUser(user);
    }
}

//创建用户窗体
void UserFrameList::addUser(std::shared_ptr<User> user)
{
    UserLoginWidget *widget = new UserLoginWidget(this);
    widget->setFixedSize(QSize(UserFrameWidth, UserFrameHeight));
    connect(widget, &UserLoginWidget::clicked, this, &UserFrameList::onUserClicked);
    widget->setUserAvatarSize(UserLoginWidget::AvatarSmallSize);
    widget->setAvatar(user->avatarPath());
    widget->setName(user->name());
    widget->setWidgetShowType(UserLoginWidget::UserFrameType);
    widget->setIsLogin(user->isLogin());

    m_userLoginWidgets[user->uid()] = widget;
    m_folwLayout->addWidget(widget);
}

//删除用户
void UserFrameList::removeUser(const uint uid)
{
    UserLoginWidget *widget = m_userLoginWidgets[uid];
    m_userLoginWidgets.remove(uid);
    widget->deleteLater();
}

//点击用户
void UserFrameList::onUserClicked()
{
    UserLoginWidget *widget = static_cast<UserLoginWidget *>(sender());
    if (!widget) return;

    emit requestSwitchUser(m_model->findUserByUid(m_userLoginWidgets.key(widget)));

    emit clicked();
}

void UserFrameList::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    //处理窗体数量小于5个时的居中显示，取 窗体数量*窗体宽度 和 最大宽度 的较小值，设置为m_centerWidget的宽度
    int width = qMin((UserFrameWidth + UserFrameSpaceing) * UserFrameColCount, m_userLoginWidgets.size() * (UserFrameWidth + UserFrameSpaceing));
    m_centerWidget->setFixedWidth(width);
    m_scrollArea->setFixedWidth(width + 10);
}

void UserFrameList::mouseReleaseEvent(QMouseEvent *event)
{
    emit clicked();
    hide();

    return QWidget::mouseReleaseEvent(event);
}

void UserFrameList::hideEvent(QHideEvent *event)
{
    releaseKeyboard();

    return QWidget::hideEvent(event);
}

void UserFrameList::initUI()
{
    setFocusPolicy(Qt::NoFocus);
    m_folwLayout = new DFlowLayout;
    m_folwLayout->setFlow(QListView::LeftToRight);
    m_folwLayout->setMargin(0);
    m_folwLayout->setSpacing(UserFrameSpaceing);

    m_centerWidget = new QWidget;
    m_centerWidget->setLayout(m_folwLayout);
    //m_centerWidget设置固定宽度，实现m_folwLayout布局插入UserLoginWidget时，一行最多插入5个
    m_centerWidget->setFixedWidth((UserFrameWidth + UserFrameSpaceing) * UserFrameColCount);

    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    //m_scrollArea纵向滑动条在需要时显示，横向不显示
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setWidget(m_centerWidget);
    //设置m_scrollArea固定宽度为两行窗体高，控制滑动条的显示
    m_scrollArea->setFixedHeight((UserFrameHeight + UserFrameSpaceing) * UserFrameRowCount);

    QVBoxLayout *mainLayout;
    mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_scrollArea, 0, Qt::AlignCenter);
    setLayout(mainLayout);
}
