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
#include <gtest/gtest.h>

#include "userinfo.h"

class UT_Interface : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;
};

void UT_Interface::SetUp()
{

}

void UT_Interface::TearDown()
{

}

TEST_F(UT_Interface, user)
{
    std::shared_ptr<NativeUser> nativeUser(new NativeUser("/com/deepin/daemon/Accounts/User"+QString::number((getuid()))));
    ASSERT_TRUE(nativeUser->getUserInter());
    EXPECT_TRUE(nativeUser->getUserInter()->greeterBackground().isEmpty());
    EXPECT_TRUE(nativeUser->getUserInter()->uid().isEmpty());
    EXPECT_TRUE(nativeUser->getUserInter()->userName().isEmpty());
    EXPECT_TRUE(nativeUser->getUserInter()->homeDir().isEmpty());
    EXPECT_FALSE(nativeUser->getUserInter()->desktopBackgrounds().size() > 0);
    EXPECT_TRUE(nativeUser->getUserInter()->isValid());
    EXPECT_FALSE(nativeUser->getUserInter()->use24HourFormat());

    std::shared_ptr<ADDomainUser> addomainUser(new ADDomainUser(getuid()));
    QString name = nativeUser->name();
    nativeUser->getUserInter()->UserNameChanged(name.append("marks"));
    EXPECT_EQ(nativeUser->displayName(), name);
    addomainUser->setUserName(name);

    QString displayname = nativeUser->displayName();
    addomainUser->displayNameChanged(displayname.append("aaaaaa"));
    EXPECT_EQ(nativeUser->name(), name);
    addomainUser->setUserDisplayName(displayname);

    uid_t id = getuid();
    addomainUser->setUid(++id);
}
