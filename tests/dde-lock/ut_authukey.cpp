#include "authcommon.h"
#include "auth_ukey.h"

#include <QPaintEvent>
#include <QTest>

#include <gtest/gtest.h>

class UT_AuthUKey : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    AuthUKey *m_authUkey;
};

void UT_AuthUKey::SetUp()
{
    m_authUkey = new AuthUKey();
}
void UT_AuthUKey::TearDown()
{
    delete m_authUkey;
}

TEST_F(UT_AuthUKey, BasicTest)
{
    m_authUkey->lineEditText();
    m_authUkey->setLimitsInfo(LimitsInfo());
}

TEST_F(UT_AuthUKey, AuthResultTest)
{
    m_authUkey->setAuthStatus(13, "test");
    m_authUkey->setAuthStatus(AuthCommon::StatusCodeSuccess, "Success");
    m_authUkey->setAuthStatus(AuthCommon::StatusCodeFailure, "Failure");
    m_authUkey->setAuthStatus(AuthCommon::StatusCodeCancel, "Cancel");
    m_authUkey->setAuthStatus(AuthCommon::StatusCodeTimeout, "Timeout");
    m_authUkey->setAuthStatus(AuthCommon::StatusCodeError, "Error");
    m_authUkey->setAuthStatus(AuthCommon::StatusCodeVerify, "Verif");
    m_authUkey->setAuthStatus(AuthCommon::StatusCodeException, "Exception");
    m_authUkey->setAuthStatus(AuthCommon::StatusCodePrompt, "Prompt");
    m_authUkey->setAuthStatus(AuthCommon::StatusCodeStarted, "Started");
    m_authUkey->setAuthStatus(AuthCommon::StatusCodeEnded, "Ended");
    m_authUkey->setAuthStatus(AuthCommon::StatusCodeLocked, "Locked");
    m_authUkey->setAuthStatus(AuthCommon::StatusCodeRecover, "Recover");
    m_authUkey->setAuthStatus(AuthCommon::StatusCodeUnlocked, "Unlocked");
}

TEST_F(UT_AuthUKey, CapsStatusTest)
{
    m_authUkey->setCapsLockVisible(true);
    m_authUkey->setCapsLockVisible(false);
}

TEST_F(UT_AuthUKey, AnimationStateTest)
{
    m_authUkey->setAnimationStatus(true);
    m_authUkey->setAnimationStatus(false);
}

TEST_F(UT_AuthUKey, LineEditInfoTest)
{
    m_authUkey->setLineEditInfo("Alert", AuthModule::AlertText);
    m_authUkey->setLineEditInfo("Input", AuthModule::InputText);
    m_authUkey->setLineEditInfo("PlaceHolder", AuthModule::PlaceHolderText);
}
