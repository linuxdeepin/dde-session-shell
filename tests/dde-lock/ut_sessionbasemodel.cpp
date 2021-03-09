#include "sessionbasemodel.h"
#include "userinfo.h"

#include <QSignalSpy>

#include <gtest/gtest.h>

class UT_SessionBaseModel : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    SessionBaseModel *m_sessionBaseModel;
    std::shared_ptr<NativeUser> m_nativeUser;
};

void UT_SessionBaseModel::SetUp()
{
    m_sessionBaseModel = new SessionBaseModel(SessionBaseModel::AuthType::LockType);
    std::shared_ptr<NativeUser> nativeUser(new NativeUser("/com/deepin/daemon/Accounts/User1001"));
}

void UT_SessionBaseModel::TearDown()
{
    delete m_sessionBaseModel;
}

TEST_F(UT_SessionBaseModel, init)
{
    ASSERT_TRUE(m_sessionBaseModel);
    EXPECT_EQ(m_sessionBaseModel->currentType(), SessionBaseModel::AuthType::LockType);
    m_sessionBaseModel->setCurrentUser(m_nativeUser);
    QSignalSpy signalSpy(m_sessionBaseModel, SIGNAL(currentUserChanged(std::shared_ptr<User>)));
    EXPECT_GT(signalSpy.count(), 0);
    EXPECT_EQ(m_sessionBaseModel->currentUser(), m_nativeUser);
}
