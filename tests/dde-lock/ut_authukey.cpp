#include "authcommon.h"
#include "authukey.h"

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
    m_authUkey->setAuthResult(13, "test");
    m_authUkey->setAuthResult(AuthCommon::StatusCodeSuccess, "Success");
    m_authUkey->setAuthResult(AuthCommon::StatusCodeFailure, "Failure");
    m_authUkey->setAuthResult(AuthCommon::StatusCodeCancel, "Cancel");
    m_authUkey->setAuthResult(AuthCommon::StatusCodeTimeout, "Timeout");
    m_authUkey->setAuthResult(AuthCommon::StatusCodeError, "Error");
    m_authUkey->setAuthResult(AuthCommon::StatusCodeVerify, "Verif");
    m_authUkey->setAuthResult(AuthCommon::StatusCodeException, "Exception");
    m_authUkey->setAuthResult(AuthCommon::StatusCodePrompt, "Prompt");
    m_authUkey->setAuthResult(AuthCommon::StatusCodeStarted, "Started");
    m_authUkey->setAuthResult(AuthCommon::StatusCodeEnded, "Ended");
    m_authUkey->setAuthResult(AuthCommon::StatusCodeLocked, "Locked");
    m_authUkey->setAuthResult(AuthCommon::StatusCodeRecover, "Recover");
    m_authUkey->setAuthResult(AuthCommon::StatusCodeUnlocked, "Unlocked");
}

TEST_F(UT_AuthUKey, CapsStatusTest)
{
    m_authUkey->setCapsStatus(true);
    m_authUkey->setCapsStatus(false);
}

TEST_F(UT_AuthUKey, AnimationStateTest)
{
    m_authUkey->setAnimationState(true);
    m_authUkey->setAnimationState(false);
}

TEST_F(UT_AuthUKey, LineEditInfoTest)
{
    m_authUkey->setLineEditInfo("Alert", AuthModule::AlertText);
    m_authUkey->setLineEditInfo("Input", AuthModule::InputText);
    m_authUkey->setLineEditInfo("PlaceHolder", AuthModule::PlaceHolderText);
}

TEST_F(UT_AuthUKey, NumLockStatusTest)
{
    m_authUkey->setNumLockStatus("");
}

TEST_F(UT_AuthUKey, KeyboardButtonTest)
{
    m_authUkey->setKeyboardButtonVisible(true);
    m_authUkey->setKeyboardButtonInfo("cn;");
}
