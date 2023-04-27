// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "login_plugin_util.h"

#include "login_plugin.h"
#include "plugin_manager.h"

int LPUtil::loginType()
{
    auto *plugin = PluginManager::instance()->getLoginPlugin();
    if (!plugin) {
        return LoginPlugin::CustomLoginType::CLT_Default;
    }

    return plugin->loginType();
}

int LPUtil::updateLoginType()
{
    auto plugin = PluginManager::instance()->getLoginPlugin();
    if (!plugin) {
        return LoginPlugin::CustomLoginType::CLT_Default;
    }
    return plugin->updateLoginType();
}

int LPUtil::sessionTimeout()
{
    auto plugin = PluginManager::instance()->getLoginPlugin();
    if (!plugin) {
        return DEFAULT_SESSION_TIMEOUT;
    }
    return plugin->sessionTimeout();
}

bool LPUtil::hasSecondLevel(const QString &user)
{
    auto plugin = PluginManager::instance()->getLoginPlugin();
    if (!plugin) {
        return false;
    }
    return plugin->hasSecondLevel(user);
}
