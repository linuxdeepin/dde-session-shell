#include "auth_module.h"
#include "authcommon.h"

#include <gtest/gtest.h>

class UT_AuthModule : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    AuthModule *m_authModule;
};

void UT_AuthModule::SetUp()
{
    m_authModule = new AuthModule(AuthCommon::AT_None);
}

void UT_AuthModule::TearDown()
{
    delete m_authModule;
}

TEST_F(UT_AuthModule, BasicTest)
{
    m_authModule->authStatus();
    m_authModule->authType();
    m_authModule->setAnimationStatus(false);
    m_authModule->setAuthStatus(0, "test");
    // m_authModule->setAuthStatus("");
    m_authModule->setLimitsInfo(LimitsInfo());
}
