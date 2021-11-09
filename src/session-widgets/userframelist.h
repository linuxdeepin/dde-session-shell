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

#ifndef USERFRAMELIST_H
#define USERFRAMELIST_H

#include <dflowlayout.h>

#include <QMap>
#include <QPointer>

#include <memory>

class UserWidget;
class User;
class SessionBaseModel;
class FrameDataBind;
class UserLoginWidget;
class QVBoxLayout;
class QScrollArea;

DWIDGET_USE_NAMESPACE

class UserFrameList : public QWidget
{
    Q_OBJECT
public:
    explicit UserFrameList(QWidget *parent = nullptr);
    void setModel(SessionBaseModel *model);
    void setFixedSize(const QSize &size);
    void updateLayout();

signals:
    void requestSwitchUser(std::shared_ptr<User> user);
    void clicked();

protected:
    void mouseReleaseEvent(QMouseEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void initUI();
    void handlerBeforeAddUser(std::shared_ptr<User> user);
    void addUser(const std::shared_ptr<User> user);
    void removeUser(const std::shared_ptr<User> user);
    void onUserClicked();
    void switchNextUser();
    void switchPreviousUser();
    void onOtherPageChanged(const QVariant &value);

private:
    QScrollArea *m_scrollArea;
    DFlowLayout *m_flowLayout;
    QList<UserWidget *> m_loginWidgets;
    QMap<uid_t, UserWidget *> m_userWidgets;
    QPointer<UserWidget> currentSelectedUser;
    SessionBaseModel *m_model;
    FrameDataBind *m_frameDataBind;
    QWidget *m_centerWidget;
    int m_colCount;
    int m_rowCount;
};

#endif // USERFRAMELIST_H
