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
    , m_authCallback(new AuthCallback)
    , m_module(nullptr)
{
    setObjectName(QStringLiteral("AuthCutom"));
    setAccessibleName(QStringLiteral("AuthCutom"));
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
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    m_authCallback->app_data = this;
    m_authCallback->callbackFun = AuthCustom::authCallBack;
    m_module->setAuthFinishedCallback(m_authCallback);

    if (m_module->content()->parent() == nullptr) {
        mainLayout->addWidget(m_module->content());
    }
}

void AuthCustom::authCallBack(const AuthCallbackData *callbackData, void *app_data)
{
    qDebug() << "AuthCustom::authCallBack";
    if (callbackData && callbackData->result == 1) {
        emit static_cast<AuthCustom *>(app_data)->requestAuthenticate();
        emit static_cast<AuthCustom *>(app_data)->authFinished(AuthCommon::AS_Success);
    }
}
