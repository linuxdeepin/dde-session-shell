#include "lockcontent.h"
#include "sessionbasemodel.h"
#include "userframelist.h"
#include "userinfo.h"
#include "userlogininfo.h"

#include <QDebug>
#include <QTest>

#include <gtest/gtest.h>
#include <pwd.h>

class UT_UserLoginInfo : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    SessionBaseModel *m_model;
    UserLoginInfo *m_userLoginInfo;
};

void UT_UserLoginInfo::SetUp()
{
    m_model = new SessionBaseModel(SessionBaseModel::AuthType::LightdmType);
    std::shared_ptr<User> user_ptr(new User);
    m_model->updateCurrentUser(user_ptr);

    m_userLoginInfo = new UserLoginInfo(m_model);
}

void UT_UserLoginInfo::TearDown()
{
    delete m_model;
    delete m_userLoginInfo;
}

TEST_F(UT_UserLoginInfo, init)
{
    m_userLoginInfo->updateLocale();
    EXPECT_NE(nullptr, m_userLoginInfo->getUserLoginWidget());
    m_userLoginInfo->hideKBLayout();
    m_userLoginInfo->abortConfirm(false);
    m_userLoginInfo->beforeUnlockAction(true);
    m_userLoginInfo->updateLoginContent();
    m_userLoginInfo->getUserFrameList()->setModel(m_model);
    m_userLoginInfo->userLockChanged(false);
    std::shared_ptr<User> user_ptr(new User);
    m_userLoginInfo->receiveSwitchUser(user_ptr);
}
