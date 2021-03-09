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
#include <gtest/gtest.h>

using UserInter = com::deepin::daemon::accounts::User;

class UT_Interface : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    UserInter *m_userInter;
};

void UT_Interface::SetUp()
{
    m_userInter = new UserInter("com.deepin.daemon.Accounts", "/com/deepin/daemon/Accounts/User1001", QDBusConnection::systemBus(), nullptr);
}

void UT_Interface::TearDown()
{
    delete m_userInter;
}

TEST_F(UT_Interface, user)
{
    ASSERT_TRUE(m_userInter);
    EXPECT_FALSE(m_userInter->greeterBackground().isEmpty());
    EXPECT_FALSE(m_userInter->uid().isEmpty());
    EXPECT_FALSE(m_userInter->userName().isEmpty());
    EXPECT_FALSE(m_userInter->homeDir().isEmpty());
    EXPECT_TRUE(m_userInter->desktopBackgrounds().size() > 0);
    EXPECT_TRUE(m_userInter->isValid());
}
