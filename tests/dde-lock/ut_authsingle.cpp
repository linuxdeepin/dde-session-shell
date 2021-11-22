#include "authcommon.h"
#include "auth_single.h"

#include <gtest/gtest.h>

class UT_AuthSingle : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    AuthSingle *m_authSingle;
};

void UT_AuthSingle::SetUp()
{
    m_authSingle = new AuthSingle;
}

void UT_AuthSingle::TearDown()
{
    delete m_authSingle;
}

TEST_F(UT_AuthSingle, basic)
{
    m_authSingle->lineEditText();
    m_authSingle->setLimitsInfo(LimitsInfo());
}

TEST_F(UT_AuthSingle, AuthResultTest)
{
    m_authSingle->setAuthStatus(INT_MAX, "default");
    m_authSingle->setAuthStatus(AuthCommon::AS_Success, "Success");
    m_authSingle->setAuthStatus(AuthCommon::AS_Failure, "Failure");
    m_authSingle->setAuthStatus(AuthCommon::AS_Cancel, "Cancel");
    m_authSingle->setAuthStatus(AuthCommon::AS_Timeout, "Timeout");
    m_authSingle->setAuthStatus(AuthCommon::AS_Error, "Error");
    m_authSingle->setAuthStatus(AuthCommon::AS_Verify, "Verify");
    m_authSingle->setAuthStatus(AuthCommon::AS_Exception, "Exception");
    m_authSingle->setAuthStatus(AuthCommon::AS_Prompt, "Prompt");
    m_authSingle->setAuthStatus(AuthCommon::AS_Started, "Started");
    m_authSingle->setAuthStatus(AuthCommon::AS_Ended, "Ended");
    m_authSingle->setAuthStatus(AuthCommon::AS_Locked, "Locked");
    m_authSingle->setAuthStatus(AuthCommon::AS_Recover, "Recover");
    m_authSingle->setAuthStatus(AuthCommon::AS_Unlocked, "Unlocked");
}

TEST_F(UT_AuthSingle, CapsStatusTest)
{
    m_authSingle->setCapsLockVisible(true);
    m_authSingle->setCapsLockVisible(false);
}

TEST_F(UT_AuthSingle, AnimationStateTest)
{
    m_authSingle->setAnimationStatus(true);
    m_authSingle->setAnimationStatus(false);
}

TEST_F(UT_AuthSingle, LineEditInfoTest)
{
    m_authSingle->setLineEditInfo("Alert", AuthModule::AlertText);
    m_authSingle->setLineEditInfo("Input", AuthModule::InputText);
    m_authSingle->setLineEditInfo("PlaceHolder", AuthModule::PlaceHolderText);
}

TEST_F(UT_AuthSingle, KeyboardButtonTest)
{
    m_authSingle->setKeyboardButtonVisible(true);
    m_authSingle->setKeyboardButtonInfo("cn;");
}
