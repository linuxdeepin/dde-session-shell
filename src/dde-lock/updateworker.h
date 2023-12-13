// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UPDATECTRL_H
#define UPDATECTRL_H

#include <com_deepin_lastore_jobmanager.h>
#include <com_deepin_system_systempower.h>

using ManagerInter = com::deepin::lastore::Manager;
using PowerInter = com::deepin::system::Power;

class UpdateWorker : public QObject
{
    Q_OBJECT

public:
    static UpdateWorker *instance() {
        static UpdateWorker *pIns = nullptr;
        if (!pIns) {
            pIns = new UpdateWorker();
        }
        return pIns;
    };

    void doUpdate(bool powerOff);

Q_SIGNALS:
    void requestExitUpdating();

private:
    explicit UpdateWorker(QObject *parent = nullptr);

    bool checkPower();
    void prepareFullScreenUpgrade(bool powerOff);
};

#endif // UPDATECTRL_H
