#include "authcommon.h"
#include "auth_fingerprint.h"

#include <QTest>

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
    m_authFingerprint->setAuthStatus(0, "test");
    m_authFingerprint->setAnimationStatus(true);
    m_authFingerprint->setLimitsInfo(LimitsInfo());
    QTest::mouseRelease(m_authFingerprint, Qt::MouseButton::RightButton, Qt::KeyboardModifier::NoModifier, QPoint(0, 0));
}

TEST_F(UT_AuthFingerprint, AuthResultTest)
{
    m_authFingerprint->setAuthStatus(INT_MAX, "default");
    m_authFingerprint->setAuthStatus(AuthCommon::StatusCodeSuccess, "Success");
    m_authFingerprint->setAuthStatus(AuthCommon::StatusCodeFailure, "Failure");
    m_authFingerprint->setAuthStatus(AuthCommon::StatusCodeCancel, "Cancel");
    m_authFingerprint->setAuthStatus(AuthCommon::StatusCodeTimeout, "Timeout");
    m_authFingerprint->setAuthStatus(AuthCommon::StatusCodeError, "Error");
    m_authFingerprint->setAuthStatus(AuthCommon::StatusCodeVerify, "Verify");
    m_authFingerprint->setAuthStatus(AuthCommon::StatusCodeException, "Exception");
    m_authFingerprint->setAuthStatus(AuthCommon::StatusCodePrompt, "Prompt");
    m_authFingerprint->setAuthStatus(AuthCommon::StatusCodeStarted, "Started");
    m_authFingerprint->setAuthStatus(AuthCommon::StatusCodeEnded, "Ended");
    m_authFingerprint->setAuthStatus(AuthCommon::StatusCodeLocked, "Locked");
    m_authFingerprint->setAuthStatus(AuthCommon::StatusCodeRecover, "Recover");
    m_authFingerprint->setAuthStatus(AuthCommon::StatusCodeUnlocked, "Unlocked");
}
