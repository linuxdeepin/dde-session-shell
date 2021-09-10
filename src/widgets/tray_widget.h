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

#ifndef TRAYWIDGET_H
#define TRAYWIDGET_H

#include "tray_module_interface.h"

#include <DFloatingButton>

#include <QHBoxLayout>
#include <QMap>
#include <QWidget>

DWIDGET_USE_NAMESPACE

namespace dss {

class TrayWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TrayWidget(QWidget *parent = nullptr);

    void init();

    void setPowerButtonVisible(const bool visible);
    void setSessionSwitchButtonVisible(const bool visible);
    void setUserSwitchButtonVisible(const bool visible);
    void setVirtualKeyboardButtonVisible(const bool visible);

signals:
    void requestShowModule(const QString &);

public slots:
    void addModule(module::BaseModuleInterface *module);
    void removeModule(module::BaseModuleInterface *module);

private:
    void initPowerModule();
    void initSessionSwitchModule();
    void initUserSwitchModule();
    void initVirtualKeyboardModule();

    void updateLayout();

private:
    QHBoxLayout *m_mainLayout;
    QMap<QString, QWidget *> m_modules;
    DFloatingButton *m_powerButton;
    DFloatingButton *m_sessionSwitchButton;
    DFloatingButton *m_userSwitchButton;
    DFloatingButton *m_virtualKeyboardButton;
};

} // namespace dss
#endif // TRAYWIDGET_H
