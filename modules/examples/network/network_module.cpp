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

#include "network_module.h"

#include <QWidget>

namespace dss {
namespace module {

NetworkModule::NetworkModule(QObject *parent)
    : QObject(parent)
    , m_networkWidget(nullptr)
{
    setObjectName(QStringLiteral("NetworkModule"));
}

NetworkModule::~NetworkModule()
{
    if (m_networkWidget) {
        delete m_networkWidget;
    }
}

void NetworkModule::init()
{
    initUI();
}

void NetworkModule::initUI()
{
    if (m_networkWidget) {
        return;
    }
    m_networkWidget = new QWidget;
    m_networkWidget->setAccessibleName(QStringLiteral("NetworkWidget"));
    m_networkWidget->setFixedSize(200, 100);
}

} // namespace module
} // namespace dss
