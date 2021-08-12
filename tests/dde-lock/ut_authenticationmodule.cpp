#include "authcommon.h"
#include "authenticationmodule.h"

#include <gtest/gtest.h>

class UT_AuthenticationModule : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    AuthenticationModule *m_passwordAuth;
    AuthenticationModule *m_fingerprintAuth;
    AuthenticationModule *m_ukeyAuth;
    AuthenticationModule *m_activeDirectoryAuth;
    AuthenticationModule *m_defaultAuth;
};

void UT_AuthenticationModule::SetUp()
{
    m_passwordAuth = new AuthenticationModule(AuthCommon::AuthTypePassword);
    m_fingerprintAuth = new AuthenticationModule(AuthCommon::AuthTypeFingerprint);
    m_ukeyAuth = new AuthenticationModule(AuthCommon::AuthTypeUkey);
    m_activeDirectoryAuth = new AuthenticationModule(AuthCommon::AuthTypeActiveDirectory);
    m_defaultAuth = new AuthenticationModule(AuthCommon::AuthTypeNone);
}

void UT_AuthenticationModule::TearDown()
{
    delete m_passwordAuth;
    delete m_fingerprintAuth;
    delete m_ukeyAuth;
    delete m_activeDirectoryAuth;
    delete m_defaultAuth;
}

TEST_F(UT_AuthenticationModule, BasicTest)
{
    m_passwordAuth->lineEditText();
    m_passwordAuth->getAuthStatus();
    m_passwordAuth->setKeyboardButtonVisible(true);
    m_passwordAuth->setKeyboardButtontext("cn");
    m_passwordAuth->setAnimationState(true);
    m_passwordAuth->setAnimationState(false);
    m_passwordAuth->setAuthStatus("");
    m_passwordAuth->setCapsStatus(false);
    m_passwordAuth->setLimitsInfo(AuthenticationModule::LimitsInfo());
    // m_passwordAuth->setNumLockStatus("");
    m_passwordAuth->setKeyboardButtonInfo("cn;");
    m_passwordAuth->lineEditIsEnable();
    m_passwordAuth->setLineEditBkColor(true);

    m_fingerprintAuth->lineEditText();
    m_fingerprintAuth->setText("test");
}

TEST_F(UT_AuthenticationModule, AuthResultTest)
{
    m_passwordAuth->setAuthResult(INT_MAX, "default");
    m_passwordAuth->setAuthResult(AuthCommon::StatusCodeSuccess, "Success");
    m_passwordAuth->setAuthResult(AuthCommon::StatusCodeFailure, "Failure");
    m_passwordAuth->setAuthResult(AuthCommon::StatusCodeCancel, "Cancel");
    m_passwordAuth->setAuthResult(AuthCommon::StatusCodeTimeout, "Timeout");
    m_passwordAuth->setAuthResult(AuthCommon::StatusCodeError, "Error");
    m_passwordAuth->setAuthResult(AuthCommon::StatusCodeVerify, "Verify");
    m_passwordAuth->setAuthResult(AuthCommon::StatusCodeException, "Exception");
    m_passwordAuth->setAuthResult(AuthCommon::StatusCodePrompt, "Prompt");
    m_passwordAuth->setAuthResult(AuthCommon::StatusCodeStarted, "Started");
    m_passwordAuth->setAuthResult(AuthCommon::StatusCodeEnded, "Ended");
    m_passwordAuth->setAuthResult(AuthCommon::StatusCodeLocked, "Locked");
    m_passwordAuth->setAuthResult(AuthCommon::StatusCodeRecover, "Recover");
    m_passwordAuth->setAuthResult(AuthCommon::StatusCodeUnlocked, "Unlocked");

    m_fingerprintAuth->setAuthResult(AuthCommon::StatusCodeSuccess, "Success");
    m_fingerprintAuth->setAuthResult(AuthCommon::StatusCodeFailure, "Failure");
    m_fingerprintAuth->setAuthResult(AuthCommon::StatusCodeCancel, "Cancel");
    m_fingerprintAuth->setAuthResult(AuthCommon::StatusCodeTimeout, "Timeout");
    m_fingerprintAuth->setAuthResult(AuthCommon::StatusCodeError, "Error");
    m_fingerprintAuth->setAuthResult(AuthCommon::StatusCodeVerify, "Verify");
    m_fingerprintAuth->setAuthResult(AuthCommon::StatusCodeException, "Exception");
    m_fingerprintAuth->setAuthResult(AuthCommon::StatusCodePrompt, "Prompt");
    m_fingerprintAuth->setAuthResult(AuthCommon::StatusCodeStarted, "Started");
    m_fingerprintAuth->setAuthResult(AuthCommon::StatusCodeEnded, "Ended");
    m_fingerprintAuth->setAuthResult(AuthCommon::StatusCodeLocked, "Locked");
    m_fingerprintAuth->setAuthResult(AuthCommon::StatusCodeRecover, "Recover");
    m_fingerprintAuth->setAuthResult(AuthCommon::StatusCodeUnlocked, "Unlocked");

    m_ukeyAuth->setAuthResult(AuthCommon::StatusCodeSuccess, "Success");
    m_ukeyAuth->setAuthResult(AuthCommon::StatusCodeFailure, "Failure");
    m_ukeyAuth->setAuthResult(AuthCommon::StatusCodeCancel, "Cancel");
    m_ukeyAuth->setAuthResult(AuthCommon::StatusCodeTimeout, "Timeout");
    m_ukeyAuth->setAuthResult(AuthCommon::StatusCodeError, "Error");
    m_ukeyAuth->setAuthResult(AuthCommon::StatusCodeVerify, "Verify");
    m_ukeyAuth->setAuthResult(AuthCommon::StatusCodeException, "Exception");
    m_ukeyAuth->setAuthResult(AuthCommon::StatusCodePrompt, "Prompt");
    m_ukeyAuth->setAuthResult(AuthCommon::StatusCodeStarted, "Started");
    m_ukeyAuth->setAuthResult(AuthCommon::StatusCodeEnded, "Ended");
    m_ukeyAuth->setAuthResult(AuthCommon::StatusCodeLocked, "Locked");
    m_ukeyAuth->setAuthResult(AuthCommon::StatusCodeRecover, "Recover");
    m_ukeyAuth->setAuthResult(AuthCommon::StatusCodeUnlocked, "Unlocked");
}

TEST_F(UT_AuthenticationModule, LineEditInfoTest)
{
    m_passwordAuth->setLineEditInfo("Alert", AuthenticationModule::AlertText);
    m_passwordAuth->setLineEditInfo("Input", AuthenticationModule::InputText);
    m_passwordAuth->setLineEditInfo("PlaceHolder", AuthenticationModule::PlaceHolderText);
}
