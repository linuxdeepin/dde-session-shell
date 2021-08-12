#include "useravatar.h"

#include <gtest/gtest.h>

class UT_UserAvatar : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    UserAvatar *m_avatar;
};

void UT_UserAvatar::SetUp()
{
    m_avatar = new UserAvatar;
}

void UT_UserAvatar::TearDown()
{
    delete m_avatar;
}

TEST_F(UT_UserAvatar, BasicTest)
{
    m_avatar->setIcon("/usr/share/backgrounds/default_background.jpg");
    m_avatar->setAvatarSize(UserAvatar::AvatarSmallSize);
    m_avatar->setDisabled(false);
    m_avatar->setBorderSelectedColor(QColor());
    m_avatar->borderSelectedColor();
    m_avatar->borderColor();
    m_avatar->setBorderWidth(10);
    m_avatar->borderWidth();
    m_avatar->setSelected(true);
    m_avatar->setDeleteable(true);
    m_avatar->deleteable();
    m_avatar->setColor(QColor());
}
