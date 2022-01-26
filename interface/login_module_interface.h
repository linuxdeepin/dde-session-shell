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

#ifndef LOGINMODULEINTERFACE_H
#define LOGINMODULEINTERFACE_H

#include "base_module_interface.h"

namespace dss {
namespace module {

/**
 * @brief 认证插件需要传回的数据
 */
struct AuthCallbackData{
    int result;          // 认证结果
    std::string account; // 账户
    std::string token;   // 令牌
    std::string message; // 提示消息
    std::string json;    // 预留数据
};

/**
 * @brief 回调函数
 * AuthData: 需要传回的数据
 * void *: ContainerPtr
 */
using AuthCallbackFun = void (*)(const AuthCallbackData *, void *);

/**
 * @brief 插件回调相关
 */
struct AuthCallback {
    void *app_data;                // 插件无需关注
    AuthCallbackFun callbackFun;   // 回调函数
};

class LoginModuleInterface : public BaseModuleInterface
{
public:
    /**
     * @brief 模块的类型
     * @return ModuleType
     */
    ModuleType type() const override { return LoginType; }

    /**
     * @brief 模块图标的路径
     * @return std::string
     */
    virtual std::string icon() const { return nullptr; }

    /**
     * @brief 认证完成后，需要调用回调函数 CallbackFun
     */
    virtual void setAuthFinishedCallback(AuthCallback *) = 0;
};

} // namespace module
} // namespace dss

Q_DECLARE_INTERFACE(dss::module::LoginModuleInterface, "com.deepin.dde.shell.Modules.Login")

#endif // LOGINMODULEINTERFACE_H
