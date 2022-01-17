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

#include "auth_custom.h"

#include "modules_loader.h"

#include <QBoxLayout>

using namespace dss::module;

AuthCustom::AuthCustom(QWidget *parent)
    : AuthModule(AuthCommon::AT_Custom, parent)
    , m_mainLayout(new QVBoxLayout(this))
    , m_authCallback(new AuthCallback)
    , m_module(nullptr)
{
    setObjectName(QStringLiteral("AuthCutom"));
    setAccessibleName(QStringLiteral("AuthCutom"));

    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
}

void AuthCustom::setModule(dss::module::LoginModuleInterface *module)
{
    if (m_module) {
        return;
    }
    m_module = module;

    init();
}

void AuthCustom::init()
{
    m_authCallback->app_data = this;
    m_authCallback->callbackFun = AuthCustom::authCallBack;
    m_module->setAuthFinishedCallback(m_authCallback);

    if (m_module->content()->parent() == nullptr) {
        m_mainLayout->addWidget(m_module->content());
    }
}

void AuthCustom::setAuthState(const int state, const QString &result)
{
    qDebug() << "AuthCustom::setAuthState:" << state << result;
    m_state = state;
    switch (state) {
    case AuthCommon::AS_Success:
        emit authFinished(state);
        break;
    default:
        break;
    }
}

void AuthCustom::authCallBack(const AuthCallbackData *callbackData, void *app_data)
{
    qDebug() << "AuthCustom::authCallBack";
    if (callbackData && callbackData->result == 1) {
        emit static_cast<AuthCustom *>(app_data)->requestAuthenticate();
    }
}
