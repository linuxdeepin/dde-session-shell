#include "authcommon.h"
#include "authfingerprint.h"

#include <gtest/gtest.h>

class UT_AuthFingerprint : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    AuthFingerprint *m_authFingerprint;
};

void UT_AuthFingerprint::SetUp()
{
    m_authFingerprint = new AuthFingerprint;
}

void UT_AuthFingerprint::TearDown()
{
    delete m_authFingerprint;
}

TEST_F(UT_AuthFingerprint, BasicTest)
{
    m_authFingerprint->setAuthResult(0, "test");
    m_authFingerprint->setAnimationState(true);
    m_authFingerprint->setLimitsInfo(LimitsInfo());
}

TEST_F(UT_AuthFingerprint, AuthResultTest)
{
    m_authFingerprint->setAuthResult(INT_MAX, "default");
    m_authFingerprint->setAuthResult(AuthCommon::StatusCodeSuccess, "Success");
    m_authFingerprint->setAuthResult(AuthCommon::StatusCodeFailure, "Failure");
    m_authFingerprint->setAuthResult(AuthCommon::StatusCodeCancel, "Cancel");
    m_authFingerprint->setAuthResult(AuthCommon::StatusCodeTimeout, "Timeout");
    m_authFingerprint->setAuthResult(AuthCommon::StatusCodeError, "Error");
    m_authFingerprint->setAuthResult(AuthCommon::StatusCodeVerify, "Verify");
    m_authFingerprint->setAuthResult(AuthCommon::StatusCodeException, "Exception");
    m_authFingerprint->setAuthResult(AuthCommon::StatusCodePrompt, "Prompt");
    m_authFingerprint->setAuthResult(AuthCommon::StatusCodeStarted, "Started");
    m_authFingerprint->setAuthResult(AuthCommon::StatusCodeEnded, "Ended");
    m_authFingerprint->setAuthResult(AuthCommon::StatusCodeLocked, "Locked");
    m_authFingerprint->setAuthResult(AuthCommon::StatusCodeRecover, "Recover");
    m_authFingerprint->setAuthResult(AuthCommon::StatusCodeUnlocked, "Unlocked");
}
