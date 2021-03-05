/*
 * Copyright (C) 2018 ~ 2028 Uniontech Technology Co., Ltd.
 *
 * Author:     zorowk <pengwenhao@uniontech.com>
 *
 * Maintainer: zorowk <pengwenhao@uniontech.com>
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

#include <com_deepin_daemon_accounts_user.h>

#include "unit_test.h"

/**
 * @brief UnitTest::authenticate_test  
 */
using UserInter = com::deepin::daemon::accounts::User;
TEST_F(UnitTest, accountDBus)
{
    UserInter account_dbus("com.deepin.daemon.Accounts", "/com/deepin/daemon/Accounts/User1000", QDBusConnection::systemBus(), nullptr);

    ASSERT_FALSE(account_dbus.greeterBackground().isEmpty());

    ASSERT_FALSE(account_dbus.uid().isEmpty());

    ASSERT_FALSE(account_dbus.userName().isEmpty());

    ASSERT_FALSE(account_dbus.homeDir().isEmpty());

    ASSERT_TRUE(account_dbus.desktopBackgrounds().size() > 0);

    ASSERT_TRUE(account_dbus.isValid());
}
 
