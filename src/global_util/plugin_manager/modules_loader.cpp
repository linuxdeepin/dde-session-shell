// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "modules_loader.h"
#include "plugin_manager.h"
#include "base_module_interface.h"
#include "public_func.h"

#include <QDir>
#include <QLibrary>
#include <QPluginLoader>
#include <QApplication>

const QString ModulesDir = "/usr/lib/dde-session-shell/modules";
const QString LOWEST_VERSION = "1.1.0";

ModulesLoader::ModulesLoader(QObject *parent)
    : QThread(parent)
{
}

ModulesLoader::~ModulesLoader()
{
}

ModulesLoader &ModulesLoader::instance()
{
    static ModulesLoader modulesLoader;
    return modulesLoader;
}

void ModulesLoader::run()
{
    findModule(ModulesDir);
}

void ModulesLoader::findModule(const QString &path)
{
    QDir dir(path);
    if (!dir.exists()) {
        qDebug() << path << "is not exists.";
        return;
    }
    const QFileInfoList modules = dir.entryInfoList();
    for (QFileInfo module : modules) {
        const QString path = module.absoluteFilePath();
        if (!QLibrary::isLibrary(path)) {
            continue;
        }
        qInfo() << module << "is found";
        QPluginLoader loader(path);

        // 检查兼容性
        const QString &version = loader.metaData().value("MetaData").toObject().value("api").toString();
        // 版本过低则不加载，可能会导致登录器崩溃
        if (!checkVersion(version, LOWEST_VERSION)) {
            qWarning() << "The module version is too low.";
            continue;
        }

        auto *moduleInstance = dynamic_cast<dss::module::BaseModuleInterface *>(loader.instance());
        if (!moduleInstance) {
            qWarning() << "Load plugin failed, error:" << loader.errorString();
            continue;
        }

        int loadPluginType = moduleInstance->loadPluginType();
        if (loadPluginType != dss::module::BaseModuleInterface::Load) {
            qInfo() << "The plugin dose not want to be loaded";
            continue;
        }

        if (PluginManager::instance()->contains(moduleInstance->key())) {
            qWarning() << "The plugin has been loaded";
            continue;
        }

        QObject *obj = dynamic_cast<QObject*>(moduleInstance);
        if (obj)
            obj->moveToThread(qApp->thread());

        PluginManager::instance()->addPlugin(moduleInstance, version);
    }
}
