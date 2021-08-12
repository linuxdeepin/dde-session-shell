#include "authcommon.h"
#include "authsingle.h"

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
    m_authSingle->setAuthResult(INT_MAX, "default");
    m_authSingle->setAuthResult(AuthCommon::StatusCodeSuccess, "Success");
    m_authSingle->setAuthResult(AuthCommon::StatusCodeFailure, "Failure");
    m_authSingle->setAuthResult(AuthCommon::StatusCodeCancel, "Cancel");
    m_authSingle->setAuthResult(AuthCommon::StatusCodeTimeout, "Timeout");
    m_authSingle->setAuthResult(AuthCommon::StatusCodeError, "Error");
    m_authSingle->setAuthResult(AuthCommon::StatusCodeVerify, "Verify");
    m_authSingle->setAuthResult(AuthCommon::StatusCodeException, "Exception");
    m_authSingle->setAuthResult(AuthCommon::StatusCodePrompt, "Prompt");
    m_authSingle->setAuthResult(AuthCommon::StatusCodeStarted, "Started");
    m_authSingle->setAuthResult(AuthCommon::StatusCodeEnded, "Ended");
    m_authSingle->setAuthResult(AuthCommon::StatusCodeLocked, "Locked");
    m_authSingle->setAuthResult(AuthCommon::StatusCodeRecover, "Recover");
    m_authSingle->setAuthResult(AuthCommon::StatusCodeUnlocked, "Unlocked");
}

TEST_F(UT_AuthSingle, CapsStatusTest)
{
    m_authSingle->setCapsStatus(true);
    m_authSingle->setCapsStatus(false);
}

TEST_F(UT_AuthSingle, AnimationStateTest)
{
    m_authSingle->setAnimationState(true);
    m_authSingle->setAnimationState(false);
}

TEST_F(UT_AuthSingle, LineEditInfoTest)
{
    m_authSingle->setLineEditInfo("Alert", AuthModule::AlertText);
    m_authSingle->setLineEditInfo("Input", AuthModule::InputText);
    m_authSingle->setLineEditInfo("PlaceHolder", AuthModule::PlaceHolderText);
}

TEST_F(UT_AuthSingle, NumLockStatusTest)
{
    // m_authSingle->setNumLockStatus("");
}

TEST_F(UT_AuthSingle, KeyboardButtonTest)
{
    m_authSingle->setKeyboardButtonVisible(true);
    m_authSingle->setKeyboardButtonInfo("cn;");
}
