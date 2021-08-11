#include <gtest/gtest.h>
#include <QTest>
#include <pwd.h>
#include <QDebug>
#include <greeterworkek.h>

#include "userlogininfo.h"
#include "sessionbasemodel.h"
#include "userframelist.h"
#include "userinfo.h"
#include "lockcontent.h"

class UT_UserLoginInfo : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    SessionBaseModel *m_sessionBaseModel;
    UserLoginInfo *m_userLoginInfo;
    LockContent *m_lockContent;
    GreeterWorkek *m_greeterWorker;
};

void UT_UserLoginInfo::SetUp()
{
    m_sessionBaseModel = new SessionBaseModel(SessionBaseModel::AuthType::LightdmType);
    m_greeterWorker = new GreeterWorkek(m_sessionBaseModel);
    m_lockContent = new LockContent(m_sessionBaseModel);
    m_userLoginInfo = new UserLoginInfo(m_sessionBaseModel);
    m_userLoginInfo->getUserFrameList()->setModel(m_sessionBaseModel);
}

void UT_UserLoginInfo::TearDown()
{
    delete m_sessionBaseModel;
    delete m_userLoginInfo;
    delete m_lockContent;
}

TEST_F(UT_UserLoginInfo, init)
{

}

