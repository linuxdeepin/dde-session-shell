/*
 * Copyright (C) 2011 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     justforlxz <justforlxz@outlook.com>
 *
 * Maintainer: justforlxz <justforlxz@outlook.com>
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

#ifndef LOGINCONTENT_H
#define LOGINCONTENT_H

#include "src/session-widgets/lockcontent.h"

class SessionWidget;
class LoginTipsWindow;
class LoginContent : public LockContent
{
    Q_OBJECT
public:
    explicit LoginContent(SessionBaseModel *const model, QWidget *parent = nullptr);

    void onCurrentUserChanged(std::shared_ptr<User> user) override;
    void onStatusChanged(SessionBaseModel::ModeStatus status) override;
    void pushSessionFrame();
    void pushTipsFrame();
    void popTipsFrame();

private:
    SessionWidget *m_sessionFrame;
    LoginTipsWindow *m_loginTipsWindow;
    bool m_showTipsWidget = true;
};

#endif // LOGINCONTENT_H
