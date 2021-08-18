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
    m_authSingle->setAuthStatus(AuthCommon::StatusCodeSuccess, "Success");
    m_authSingle->setAuthStatus(AuthCommon::StatusCodeFailure, "Failure");
    m_authSingle->setAuthStatus(AuthCommon::StatusCodeCancel, "Cancel");
    m_authSingle->setAuthStatus(AuthCommon::StatusCodeTimeout, "Timeout");
    m_authSingle->setAuthStatus(AuthCommon::StatusCodeError, "Error");
    m_authSingle->setAuthStatus(AuthCommon::StatusCodeVerify, "Verify");
    m_authSingle->setAuthStatus(AuthCommon::StatusCodeException, "Exception");
    m_authSingle->setAuthStatus(AuthCommon::StatusCodePrompt, "Prompt");
    m_authSingle->setAuthStatus(AuthCommon::StatusCodeStarted, "Started");
    m_authSingle->setAuthStatus(AuthCommon::StatusCodeEnded, "Ended");
    m_authSingle->setAuthStatus(AuthCommon::StatusCodeLocked, "Locked");
    m_authSingle->setAuthStatus(AuthCommon::StatusCodeRecover, "Recover");
    m_authSingle->setAuthStatus(AuthCommon::StatusCodeUnlocked, "Unlocked");
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
