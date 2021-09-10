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

#ifndef TRAYMODULEINTERFACE_H
#define TRAYMODULEINTERFACE_H

#include "base_module_interface.h"

namespace dss {
namespace module {

class TrayModuleInterface : public BaseModuleInterface
{
public:
    /**
     * @brief 模块的类型
     * @return ModuleType
     */
    ModuleType type() const override { return TrayType; }

    /**
     * @brief 用于托盘区域按钮图标
     */
    virtual QString icon() const = 0;
};

} // namespace module
} // namespace dss

Q_DECLARE_INTERFACE(dss::module::TrayModuleInterface, "com.deepin.dde.shell.Modules.Tray")

#endif // TRAYMODULEINTERFACE_H
