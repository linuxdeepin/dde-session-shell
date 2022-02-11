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

#ifndef AUTHCUTOM_H
#define AUTHCUTOM_H

#include "auth_module.h"
#include "login_module_interface.h"

#include <QVBoxLayout>

class AuthCustom : public AuthModule
{
    Q_OBJECT
public:
    explicit AuthCustom(QWidget *parent = nullptr);

    void setModule(dss::module::LoginModuleInterface *module);

    void setAuthState(const int state, const QString &result) override;

private:
    void init();
    static void authCallBack(const dss::module::AuthCallbackData *callbackData, void *app_data);

private:
    QVBoxLayout *m_mainLayout;
    dss::module::AuthCallback m_authCallback;
    dss::module::LoginModuleInterface *m_module;
};

#endif // AUTHCUTOM_H
