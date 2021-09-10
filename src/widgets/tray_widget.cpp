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

#include "tray_widget.h"

#include "modules_loader.h"

#include <DFloatingButton>

DWIDGET_USE_NAMESPACE

namespace dss {

#define BUTTON_ICON_SIZE QSize(26, 26)
#define BUTTON_SIZE QSize(52, 52)

TrayWidget::TrayWidget(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(new QHBoxLayout(this))
    , m_powerButton(nullptr)
    , m_sessionSwitchButton(nullptr)
    , m_userSwitchButton(nullptr)
    , m_virtualKeyboardButton(nullptr)
{
    setAccessibleName(QStringLiteral("TrayWidget"));

    setFocusPolicy(Qt::TabFocus);

    m_mainLayout->setContentsMargins(0, 0, 60, 0);
    m_mainLayout->setSpacing(26);
    m_mainLayout->setAlignment(Qt::AlignBottom | Qt::AlignRight);
}

void TrayWidget::init()
{
    initPowerModule();
    initSessionSwitchModule();
    initUserSwitchModule();
    initVirtualKeyboardModule();

    QHash<QString, module::BaseModuleInterface *> modules = module::ModulesLoader::instance().moduleList();
    for (module::BaseModuleInterface *module : modules.values()) {
        if (module->type() == module::BaseModuleInterface::TrayType) {
            addModule(dynamic_cast<module::TrayModuleInterface *>(module));
        }
    }

    updateLayout();
}

void TrayWidget::initPowerModule()
{
    if (m_powerButton) {
        return;
    }
    m_powerButton = new DFloatingButton(this);
    m_powerButton->setAccessibleName("PowerBtn");
    m_powerButton->setIcon(QIcon(":/img/bottom_actions/shutdown_normal.svg"));
    m_powerButton->setIconSize(BUTTON_ICON_SIZE);
    m_powerButton->setFixedSize(BUTTON_SIZE);
    m_powerButton->setAutoExclusive(true);
    m_powerButton->setBackgroundRole(DPalette::Button);

    m_modules.insert(m_powerButton->accessibleName(), m_powerButton);

    connect(m_powerButton, &DFloatingButton::clicked, this, [this] {
        emit requestShowModule(QStringLiteral("PowerModule"));
    });
}

void TrayWidget::initSessionSwitchModule()
{
    if (m_sessionSwitchButton) {
        return;
    }
    m_sessionSwitchButton = new DFloatingButton(this);
    m_sessionSwitchButton->setAccessibleName(QStringLiteral("SessionSwitchButton"));
    m_sessionSwitchButton->setIconSize(BUTTON_ICON_SIZE);
    m_sessionSwitchButton->setFixedSize(BUTTON_SIZE);
    m_sessionSwitchButton->setAutoExclusive(true);
    m_sessionSwitchButton->setBackgroundRole(DPalette::Button);

    m_modules.insert(m_sessionSwitchButton->accessibleName(), m_sessionSwitchButton);

    connect(m_sessionSwitchButton, &DFloatingButton::clicked, this, [this] {
        emit requestShowModule(QStringLiteral("SessionSwitchModule"));
    });
}

void TrayWidget::initUserSwitchModule()
{
    if (m_userSwitchButton) {
        return;
    }
    m_userSwitchButton = new DFloatingButton(this);
    m_userSwitchButton->setAccessibleName("UserSwitchButton");
    m_userSwitchButton->setIcon(QIcon::fromTheme(":/img/bottom_actions/userswitch_normal.svg"));
    m_userSwitchButton->setIconSize(BUTTON_ICON_SIZE);
    m_userSwitchButton->setFixedSize(BUTTON_SIZE);
    m_userSwitchButton->setAutoExclusive(true);
    m_userSwitchButton->setBackgroundRole(DPalette::Button);

    m_modules.insert(m_userSwitchButton->accessibleName(), m_userSwitchButton);

    connect(m_userSwitchButton, &DFloatingButton::clicked, this, [this] {
        emit requestShowModule(QStringLiteral("UserSwitchModule"));
    });
}

void TrayWidget::initVirtualKeyboardModule()
{
    if (m_virtualKeyboardButton) {
        return;
    }
    m_virtualKeyboardButton = new DFloatingButton(this);
    m_virtualKeyboardButton->setAccessibleName("VirtualKeyboardButton");
    m_virtualKeyboardButton->setIcon(QIcon::fromTheme(":/img/screen_keyboard_normal.svg"));
    m_virtualKeyboardButton->setIconSize(BUTTON_ICON_SIZE);
    m_virtualKeyboardButton->setFixedSize(BUTTON_SIZE);
    m_virtualKeyboardButton->setAutoExclusive(true);
    m_virtualKeyboardButton->setBackgroundRole(DPalette::Button);

    m_modules.insert(m_virtualKeyboardButton->accessibleName(), m_virtualKeyboardButton);

    connect(m_virtualKeyboardButton, &DFloatingButton::clicked, this, [this] {
        emit requestShowModule(QStringLiteral("VirtualKeyboardModule"));
    });
}

void TrayWidget::setPowerButtonVisible(const bool visible)
{
    m_powerButton->setVisible(visible);
}

void TrayWidget::setSessionSwitchButtonVisible(const bool visible)
{
    m_sessionSwitchButton->setVisible(visible);
}

void TrayWidget::setUserSwitchButtonVisible(const bool visible)
{
    m_userSwitchButton->setVisible(visible);
}

void TrayWidget::setVirtualKeyboardButtonVisible(const bool visible)
{
    m_virtualKeyboardButton->setVisible(visible);
}

void TrayWidget::addModule(module::BaseModuleInterface *module)
{
    if (module->type() != module::BaseModuleInterface::TrayType) {
        return;
    }
    module::TrayModuleInterface *trayModule = dynamic_cast<module::TrayModuleInterface *>(module);

    DFloatingButton *button = new DFloatingButton(this);
    button->setIcon(QIcon(trayModule->icon()));
    button->setIconSize(QSize(26, 26));
    button->setFixedSize(QSize(52, 52));
    button->setAutoExclusive(true);
    button->setBackgroundRole(DPalette::Button);
    button->setIcon(QIcon(trayModule->icon()));

    m_modules.insert(trayModule->key(), trayModule->content());
    connect(button, &DFloatingButton::clicked, this, [this, trayModule] {
        emit requestShowModule(trayModule->key());
    }, Qt::UniqueConnection);

    updateLayout();
}

void TrayWidget::removeModule(module::BaseModuleInterface *module)
{
    if (module->type() != module::BaseModuleInterface::TrayType) {
        return;
    }
    module::TrayModuleInterface *trayModule = dynamic_cast<module::TrayModuleInterface *>(module);

    QMap<QString, QWidget *>::const_iterator i = m_modules.constBegin();
    while (i != m_modules.constEnd()) {
        if (i.key() == trayModule->key()) {
            m_modules.remove(i.key());
            updateLayout();
            break;
        }
        ++i;
    }
}

void TrayWidget::updateLayout()
{
    QObjectList modulesTmp = m_mainLayout->children();
    for (QWidget *module : m_modules.values()) {
        if (modulesTmp.contains(module))
            modulesTmp.removeAll(module);
        m_mainLayout->insertWidget(0, module);
    }
    for (QObject *module : modulesTmp) {
        m_mainLayout->removeWidget(qobject_cast<QWidget *>(module));
    }
}

} // namespace dss
