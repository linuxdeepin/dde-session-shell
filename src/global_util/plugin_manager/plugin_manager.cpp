// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "plugin_manager.h"
#include "login_plugin_v1.h"
#include "login_plugin_v2.h"
#include "public_func.h"
#include "dconfig_helper.h"

static PluginManager pluginManager;

PluginManager::PluginManager(QObject *parent) : QObject(parent)
{

}

PluginManager* PluginManager::instance()
{
    return &pluginManager;
}

void PluginManager::addPlugin(dss::module::BaseModuleInterface *module, const QString &version)
{
    if (!module)
        return;

    PluginBase *plugin = createPlugin(module, version);
    if (!plugin) {
        qCWarning(DDE_SHELL) << "Create plugin failed.";
        return;
    }

    const QString &key = plugin->key();
    m_plugins.insert(key, plugin);
    connect(plugin, &QObject::destroyed, this, [this, key] {
        m_plugins.remove(key);
    });
}

LoginPlugin* PluginManager::getLoginPlugin() const
{
    // 指定插件 >> 默认插件 >> 第一个插件
    const auto designatedLoginPlugin = DConfigHelper::instance()->getConfig("designatedLoginPlugin", "").toString();
    const auto defaultLoginPlugin = DConfigHelper::instance()->getConfig("defaultLoginPlugin", "").toString();
    qCInfo(DDE_SHELL) << "Designated login plugin:" << designatedLoginPlugin;
    qCInfo(DDE_SHELL) << "Default login plugin:" << defaultLoginPlugin;
    LoginPlugin* ret = nullptr;
    for (const auto &plugin : m_plugins.values()) {
        if (plugin && PluginBase::ModuleType::LoginType == plugin->type()) {
            // 第一个插件
            if (!ret)
                ret = dynamic_cast<LoginPlugin*>(plugin);
            // 指定插件
            if (!designatedLoginPlugin.isEmpty() && plugin->key() == designatedLoginPlugin) {
                ret = dynamic_cast<LoginPlugin*>(plugin);
                break;
            }
            // 默认插件
            else if (!defaultLoginPlugin.isEmpty() && plugin->key() == defaultLoginPlugin ) {
                ret = dynamic_cast<LoginPlugin*>(plugin);
            }
        }
    }

    return ret;
}


LoginPlugin *PluginManager::getFullManagedLoginPlugin() const
{
    for (const auto &plugin : m_plugins.values()) {
        if (plugin && PluginBase::ModuleType::FullManagedLoginType == plugin->type()) {
            return dynamic_cast<LoginPlugin *>(plugin);
        }
    }

    return nullptr;
}

QList<TrayPlugin*> PluginManager::trayPlugins() const
{
    QList<TrayPlugin*> list;
    for (const auto &plugin : m_plugins.values()) {
        if (plugin && PluginBase::ModuleType::TrayType == plugin->type()) {
            list.append(dynamic_cast<TrayPlugin*>(plugin));
        }
    }

    return list;
}

PluginBase* PluginManager::createPlugin(dss::module::BaseModuleInterface *module, const QString &version)
{
    if (dss::module::BaseModuleInterface::LoginType == module->type()
        || dss::module::BaseModuleInterface::FullManagedLoginType == module->type()
        || dss::module::BaseModuleInterface::IpcAssistLoginType == module->type()) {
        return createLoginPlugin(module, version);
    } else if (dss::module::BaseModuleInterface::TrayType == module->type()) {
        TrayPlugin *plugin = createTrayPlugin(module, version);
        trayPluginAdded(plugin);
        return plugin;
    }

    qCWarning(DDE_SHELL) << "This plug-in type is not recognized: " << module->type();
    return nullptr;
}

LoginPlugin* PluginManager::createLoginPlugin(dss::module::BaseModuleInterface *module, const QString &version)
{
    qCInfo(DDE_SHELL) << "Create login plugin, meta version: " << version;
    if (checkVersion(version, LoginPlugin_V2::API_VERSION)) {
        qCInfo(DDE_SHELL) << "Create LoginPluginV2";
        return new LoginPlugin_V2::LoginPluginV2(dynamic_cast<dss::module_v2::LoginModuleInterfaceV2 *>(module));
    } else if (checkVersion(version, LoginPlugin_V1::API_VERSION)){
        qCInfo(DDE_SHELL) << "Create LoginPluginV1";
        return new LoginPlugin_V1::LoginPluginV1(dynamic_cast<dss::module::LoginModuleInterface *>(module));
    }

    return nullptr;
}

TrayPlugin* PluginManager::createTrayPlugin(dss::module::BaseModuleInterface *module, const QString &version)
{
    Q_UNUSED(version)

    return new TrayPlugin(dynamic_cast<dss::module::TrayModuleInterface*>(module));
}

bool PluginManager::contains(const QString &key) const
{
    return m_plugins.contains(key);
}

PluginBase *PluginManager::findPlugin(const QString &key) const
{
    if (!m_plugins.contains(key))
        return nullptr;

    return m_plugins.value(key);
}

LoginPlugin *PluginManager::getAssistloginPlugin() const
{
    for (const auto &plugin : m_plugins.values()) {
        if (plugin && PluginBase::ModuleType::IpcAssistLoginType == plugin->type()) {
            return dynamic_cast<LoginPlugin *>(plugin);
        }
    }

    return nullptr;
}
