// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MODULES_LOADER_H
#define MODULES_LOADER_H

#include <QHash>
#include <QThread>
#include <QSharedPointer>

class ModulesLoader : public QThread
{
    Q_OBJECT
public:
    static ModulesLoader &instance();
    void setLoadLoginModule(bool loadLoginModule) { m_loadLoginModule = loadLoginModule; }

protected:
    void run() override;

private:
    explicit ModulesLoader(QObject *parent = nullptr);
    ~ModulesLoader() override;
    ModulesLoader(const ModulesLoader &) = delete;
    ModulesLoader &operator=(const ModulesLoader &) = delete;

    void findModule(const QString &path);

private:
    bool m_loadLoginModule = false;
};

#endif // MODULES_LOADER_H
