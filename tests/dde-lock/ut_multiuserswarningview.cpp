#include "multiuserswarningview.h"

#include <gtest/gtest.h>

class UT_MultiUsersWarningView : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    MultiUsersWarningView *m_multiUsersWarningView;
};

class UT_UserListItem : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    UserListItem *m_userListItem;
};

void UT_MultiUsersWarningView::SetUp()
{
    m_multiUsersWarningView = new MultiUsersWarningView(SessionBaseModel::RequireNormal);
}

void UT_MultiUsersWarningView::TearDown()
{
    delete m_multiUsersWarningView;
}

TEST_F(UT_MultiUsersWarningView, BasicTest)
{
    std::shared_ptr<User> user_ptr(new User());
    m_multiUsersWarningView->action();
    m_multiUsersWarningView->toggleButtonState();
    m_multiUsersWarningView->buttonClickHandle();
    m_multiUsersWarningView->setAcceptReason("test");
}

void UT_UserListItem::SetUp()
{
    m_userListItem = new UserListItem("", "");
}

void UT_UserListItem::TearDown()
{
    delete m_userListItem;
}

TEST_F(UT_UserListItem, BasicTest)
{
}
