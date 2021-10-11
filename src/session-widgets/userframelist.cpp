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

#include "dflowlayout.h"
#include "framedatabind.h"
#include "sessionbasemodel.h"
#include "user_widget.h"
#include "userinfo.h"

#include <QMouseEvent>
#include <QScrollArea>
#include <QScrollBar>
#include <QScroller>
#include <QVBoxLayout>

const int UserFrameSpaceing = 40;

UserFrameList::UserFrameList(QWidget *parent)
    : QWidget(parent)
    , m_scrollArea(new QScrollArea(this))
    , m_frameDataBind(FrameDataBind::Instance())
{
    setObjectName(QStringLiteral("UserFrameList"));
    setAccessibleName(QStringLiteral("UserFrameList"));

    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setFocusPolicy(Qt::StrongFocus);

    initUI();

    // 设置用户列表支持触屏滑动,TouchGesture存在bug,滑动过程中会响应其他事件,打断滑动事件,改为LeftMouseButtonGesture
    QScroller::grabGesture(m_scrollArea->viewport(), QScroller::LeftMouseButtonGesture);
    QScroller *scroller = QScroller::scroller(m_scrollArea->viewport());
    QScrollerProperties sp;
    sp.setScrollMetric(QScrollerProperties::VerticalOvershootPolicy, QScrollerProperties::OvershootWhenScrollable);
    scroller->setScrollerProperties(sp);

    std::function<void(QVariant)> function = std::bind(&UserFrameList::onOtherPageChanged, this, std::placeholders::_1);
    int index = m_frameDataBind->registerFunction("UserFrameList", function);

    connect(this, &UserFrameList::destroyed, this, [=] {
        m_frameDataBind->unRegisterFunction("UserFrameList", index);
    });
}

void UserFrameList::initUI()
{
    m_centerWidget = new QWidget;
    m_centerWidget->setAccessibleName("UserFrameListCenterWidget");

    m_flowLayout = new DFlowLayout(m_centerWidget);
    m_flowLayout->setFlow(QListView::LeftToRight);
    m_flowLayout->setContentsMargins(10, 10, 10, 10);
    m_flowLayout->setSpacing(40);

    m_scrollArea->setAccessibleName("UserFrameListCenterWidget");
    m_scrollArea->setWidget(m_centerWidget);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setFrameStyle(QFrame::NoFrame);
    m_scrollArea->setFocusPolicy(Qt::NoFocus);
    m_centerWidget->setAutoFillBackground(false);
    m_scrollArea->viewport()->setAutoFillBackground(false);
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_scrollArea, 0, Qt::AlignCenter);
}

//设置SessionBaseModel，创建用户列表窗体
void UserFrameList::setModel(SessionBaseModel *model)
{
    m_model = model;

    connect(model, &SessionBaseModel::userAdded, this, &UserFrameList::handlerBeforeAddUser);
    connect(model, &SessionBaseModel::userRemoved, this, &UserFrameList::removeUser);

    QList<std::shared_ptr<User>> userList = m_model->userList();
    for (auto user : userList) {
        handlerBeforeAddUser(user);
    }
}

void UserFrameList::setFixedSize(const QSize &size)
{
    QWidget::setFixedSize(size);
    //先确定界面大小，再根据界面大小计算用户列表区域大小
    updateLayout();
}

void UserFrameList::handlerBeforeAddUser(std::shared_ptr<User> user)
{
    if (m_model->isServerModel()) {
        if (user->isLogin() || user->type() == User::Default)
            addUser(user);
        connect(user.get(), &User::loginStatusChanged, this, [=](bool is_login) {
            if (is_login) {
                addUser(user);
            } else {
                removeUser(user);
            }
        });
    } else {
        addUser(user);
    }
}

//创建用户窗体
void UserFrameList::addUser(const std::shared_ptr<User> user)
{
    qDebug() << "UserFrameList::addUser:" << user->path();
    UserWidget *widget = new UserWidget(this);
    widget->setUser(user);
    widget->setSelected(m_model->currentUser()->uid() == user->uid());
    m_loginWidgets.append(widget);

    connect(widget, &UserWidget::clicked, this, &UserFrameList::onUserClicked);

    //多用户的情况按照其uid排序，升序排列，符合账户先后创建顺序
    m_loginWidgets.push_back(widget);
    qSort(m_loginWidgets.begin(), m_loginWidgets.end(), [=](UserWidget *w1, UserWidget *w2) {
        return (w1->uid() < w2->uid());
    });
    int index = m_loginWidgets.indexOf(widget);
    m_flowLayout->insertWidget(index, widget);

    //添加用户和删除用户时，重新计算区域大小
    updateLayout();
}

//删除用户
void UserFrameList::removeUser(const std::shared_ptr<User> user)
{
    qDebug() << "UserFrameList::removeUser:" << user->path();
    foreach (auto w, m_loginWidgets) {
        if (w->uid() == user->uid()) {
            m_loginWidgets.removeOne(w);

            if (w == currentSelectedUser) {
                currentSelectedUser = nullptr;
            }
            w->deleteLater();
            break;
        }
    }

    //添加用户和删除用户时，重新计算区域大小
    updateLayout();
}

//点击用户
void UserFrameList::onUserClicked()
{
    UserWidget *widget = static_cast<UserWidget *>(sender());
    if (!widget) return;

    currentSelectedUser = widget;
    for (int i = 0; i != m_loginWidgets.size(); ++i) {
        if (m_loginWidgets[i]->isSelected()) {
            m_loginWidgets[i]->setFastSelected(false);
        }
    }
    emit requestSwitchUser(m_model->findUserByUid(widget->uid()));
}

/**
 * @brief 切换下一个用户
 */
void UserFrameList::switchNextUser()
{
    for (int i = 0; i != m_loginWidgets.size(); ++i) {
        if (m_loginWidgets[i]->isSelected()) {
            m_loginWidgets[i]->setSelected(false);
            if (i == (m_loginWidgets.size() - 1)) {
                currentSelectedUser = m_loginWidgets.first();
                currentSelectedUser->setSelected(true);
                //处理m_scrollArea翻页显示
                m_scrollArea->verticalScrollBar()->setValue(0);
                m_frameDataBind->updateValue("UserFrameList", currentSelectedUser->uid());
            } else {
                //处理m_scrollArea翻页显示
                int selectedRight = m_loginWidgets[i]->geometry().right();
                int scrollRight = m_scrollArea->widget()->geometry().right();
                if (selectedRight + UserFrameSpaceing == scrollRight) {
                    QPoint topLeft;
                    if (m_rowCount == 1) {
                        topLeft = m_loginWidgets[i + 1]->geometry().topLeft();
                    } else {
                        topLeft = m_loginWidgets[i]->geometry().topLeft();
                    }
                    m_scrollArea->verticalScrollBar()->setValue(topLeft.y());
                }

                currentSelectedUser = m_loginWidgets[i + 1];
                currentSelectedUser->setSelected(true);
                m_frameDataBind->updateValue("UserFrameList", currentSelectedUser->uid());
            }
            break;
        }
    }
}

/**
 * @brief 切换上一个用户
 */
void UserFrameList::switchPreviousUser()
{
    for (int i = 0; i != m_loginWidgets.size(); ++i) {
        if (m_loginWidgets[i]->isSelected()) {
            m_loginWidgets[i]->setSelected(false);
            if (i == 0) {
                currentSelectedUser = m_loginWidgets.last();
                currentSelectedUser->setSelected(true);
                //处理m_scrollArea翻页显示
                m_scrollArea->verticalScrollBar()->setValue(m_scrollArea->verticalScrollBar()->maximum());
                m_frameDataBind->updateValue("UserFrameList", currentSelectedUser->uid());
            } else {
                //处理m_scrollArea翻页显示
                QPoint topLeft = m_loginWidgets[i]->geometry().topLeft();
                if (topLeft.x() == 0) {
                    m_scrollArea->verticalScrollBar()->setValue(m_loginWidgets[i - 1]->geometry().topLeft().y());
                }

                currentSelectedUser = m_loginWidgets[i - 1];
                currentSelectedUser->setSelected(true);
                m_frameDataBind->updateValue("UserFrameList", currentSelectedUser->uid());
            }
            break;
        }
    }
}

void UserFrameList::onOtherPageChanged(const QVariant &value)
{
    foreach (auto w, m_loginWidgets) {
        w->setSelected(w->uid() == value.toUInt());
    }
}

void UserFrameList::updateLayout()
{
    //处理窗体数量小于5个时的居中显示，取 窗体数量*窗体宽度 和 最大宽度 的较小值，设置为m_centerWidget的宽度
    int count = m_flowLayout->count();
    if (count < 5 && count > 0) {
        m_scrollArea->setFixedSize((UserFrameWidth + UserFrameSpaceing) * count, UserFrameHeight + 20);
    }
    if (count > 5) {
        m_scrollArea->setFixedSize((UserFrameWidth + UserFrameSpaceing) * 5, (UserFrameHeight + UserFrameSpaceing) * 2);
    }
    return;

    int maxWidth = this->width();
    int maxHeight = this->height();

    m_colCount = maxWidth / (UserFrameWidth + UserFrameSpaceing);
    m_colCount = (m_colCount > 5 && count > 5) ? 5 : count;
    m_rowCount = maxHeight / (UserFrameHeight + UserFrameSpaceing);
    m_rowCount = (m_rowCount > 2 && count > 5) ? 2 : 1;

    //fix BUG 3268
    if (m_loginWidgets.size() <= m_colCount) {
        m_rowCount = 1;
    }

    int totalHeight = (UserFrameHeight + UserFrameSpaceing) * m_rowCount;
    int totalWidth = qMin((UserFrameWidth + UserFrameSpaceing) * m_colCount, (UserFrameWidth + UserFrameSpaceing) * m_loginWidgets.size());

    m_scrollArea->setFixedHeight(totalHeight);
    m_scrollArea->setFixedWidth(totalWidth + 10);
    m_centerWidget->setFixedWidth(totalWidth);
    // setFixedWidth(totalWidth);
    // setFixedHeight(totalHeight);

    std::shared_ptr<User> user = m_model->currentUser();
    if (user.get() == nullptr) return;
    for (auto it = m_loginWidgets.constBegin(); it != m_loginWidgets.constEnd(); ++it) {
        auto login_widget = *it;

        if (login_widget->uid() == user->uid()) {
            currentSelectedUser = login_widget;
            currentSelectedUser->setSelected(true);
        } else {
            login_widget->setSelected(false);
        }
    }
}

void UserFrameList::mouseReleaseEvent(QMouseEvent *event)
{
    // 触屏点击空白处不退出用户列表界面
    if (event->source() == Qt::MouseEventSynthesizedByQt) {
        return;
    }

    emit clicked();
    hide();

    return QWidget::mouseReleaseEvent(event);
}

void UserFrameList::hideEvent(QHideEvent *event)
{
    return QWidget::hideEvent(event);
}

void UserFrameList::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Left:
        switchPreviousUser();
        break;
    case Qt::Key_Right:
        switchNextUser();
        break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        for (auto it = m_loginWidgets.constBegin(); it != m_loginWidgets.constEnd(); ++it) {
            if ((*it)->isSelected()) {
                emit(*it)->clicked();
                break;
            }
        }
        break;
    case Qt::Key_Escape:
        emit clicked();
        break;
    default:
        break;
    }
    return QWidget::keyPressEvent(event);
}

void UserFrameList::focusInEvent(QFocusEvent *event)
{
    Q_UNUSED(event)
    if (currentSelectedUser != nullptr) {
        currentSelectedUser->setSelected(true);
    }
}

void UserFrameList::focusOutEvent(QFocusEvent *event)
{
    Q_UNUSED(event)
    if (currentSelectedUser != nullptr)
        currentSelectedUser->setSelected(false);
}

void UserFrameList::resizeEvent(QResizeEvent *event)
{
    updateLayout();

    QWidget::resizeEvent(event);
}
