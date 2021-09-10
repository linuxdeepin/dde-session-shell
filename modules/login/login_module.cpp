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

#include "login_module.h"

#include <QWidget>

namespace dss {
namespace module {

LoginModule::LoginModule(QObject *parent)
    : QObject(parent)
    , m_funPtr(nullptr)
    , m_loginWidget(new QWidget)
{
    setObjectName(QStringLiteral("LoginModule"));

    initUI();
}

LoginModule::~LoginModule()
{
    delete m_loginWidget;
}

void LoginModule::initUI()
{
    m_loginWidget->setAccessibleName(QStringLiteral("LoginWidget"));
    m_loginWidget->setFixedSize(200, 100);
}

void LoginModule::setAuthFinishedCallBack(Fun funPtr)
{
    m_funPtr = funPtr;
}

} // namespace module
} // namespace dss
