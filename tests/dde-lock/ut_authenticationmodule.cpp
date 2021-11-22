#include "authcommon.h"
#include "authenticationmodule.h"

#include <QTest>

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
    m_passwordAuth = new AuthenticationModule(AuthCommon::AT_Password);
    m_fingerprintAuth = new AuthenticationModule(AuthCommon::AT_Fingerprint);
    m_ukeyAuth = new AuthenticationModule(AuthCommon::AT_Ukey);
    m_activeDirectoryAuth = new AuthenticationModule(AuthCommon::AT_ActiveDirectory);
    m_defaultAuth = new AuthenticationModule(AuthCommon::AT_None);
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
    QTest::mouseRelease(m_fingerprintAuth, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(0, 0));
}

TEST_F(UT_AuthenticationModule, AuthResultTest)
{
    m_passwordAuth->setAuthResult(INT_MAX, "default");
    m_passwordAuth->setAuthResult(AuthCommon::AS_Success, "Success");
    m_passwordAuth->setAuthResult(AuthCommon::AS_Failure, "Failure");
    m_passwordAuth->setAuthResult(AuthCommon::AS_Cancel, "Cancel");
    m_passwordAuth->setAuthResult(AuthCommon::AS_Timeout, "Timeout");
    m_passwordAuth->setAuthResult(AuthCommon::AS_Error, "Error");
    m_passwordAuth->setAuthResult(AuthCommon::AS_Verify, "Verify");
    m_passwordAuth->setAuthResult(AuthCommon::AS_Exception, "Exception");
    m_passwordAuth->setAuthResult(AuthCommon::AS_Prompt, "Prompt");
    m_passwordAuth->setAuthResult(AuthCommon::AS_Started, "Started");
    m_passwordAuth->setAuthResult(AuthCommon::AS_Ended, "Ended");
    m_passwordAuth->setAuthResult(AuthCommon::AS_Locked, "Locked");
    m_passwordAuth->setAuthResult(AuthCommon::AS_Recover, "Recover");
    m_passwordAuth->setAuthResult(AuthCommon::AS_Unlocked, "Unlocked");

    m_fingerprintAuth->setAuthResult(AuthCommon::AS_Success, "Success");
    m_fingerprintAuth->setAuthResult(AuthCommon::AS_Failure, "Failure");
    m_fingerprintAuth->setAuthResult(AuthCommon::AS_Cancel, "Cancel");
    m_fingerprintAuth->setAuthResult(AuthCommon::AS_Timeout, "Timeout");
    m_fingerprintAuth->setAuthResult(AuthCommon::AS_Error, "Error");
    m_fingerprintAuth->setAuthResult(AuthCommon::AS_Verify, "Verify");
    m_fingerprintAuth->setAuthResult(AuthCommon::AS_Exception, "Exception");
    m_fingerprintAuth->setAuthResult(AuthCommon::AS_Prompt, "Prompt");
    m_fingerprintAuth->setAuthResult(AuthCommon::AS_Started, "Started");
    m_fingerprintAuth->setAuthResult(AuthCommon::AS_Ended, "Ended");
    m_fingerprintAuth->setAuthResult(AuthCommon::AS_Locked, "Locked");
    m_fingerprintAuth->setAuthResult(AuthCommon::AS_Recover, "Recover");
    m_fingerprintAuth->setAuthResult(AuthCommon::AS_Unlocked, "Unlocked");

    m_ukeyAuth->setAuthResult(AuthCommon::AS_Success, "Success");
    m_ukeyAuth->setAuthResult(AuthCommon::AS_Failure, "Failure");
    m_ukeyAuth->setAuthResult(AuthCommon::AS_Cancel, "Cancel");
    m_ukeyAuth->setAuthResult(AuthCommon::AS_Timeout, "Timeout");
    m_ukeyAuth->setAuthResult(AuthCommon::AS_Error, "Error");
    m_ukeyAuth->setAuthResult(AuthCommon::AS_Verify, "Verify");
    m_ukeyAuth->setAuthResult(AuthCommon::AS_Exception, "Exception");
    m_ukeyAuth->setAuthResult(AuthCommon::AS_Prompt, "Prompt");
    m_ukeyAuth->setAuthResult(AuthCommon::AS_Started, "Started");
    m_ukeyAuth->setAuthResult(AuthCommon::AS_Ended, "Ended");
    m_ukeyAuth->setAuthResult(AuthCommon::AS_Locked, "Locked");
    m_ukeyAuth->setAuthResult(AuthCommon::AS_Recover, "Recover");
    m_ukeyAuth->setAuthResult(AuthCommon::AS_Unlocked, "Unlocked");
}

TEST_F(UT_AuthenticationModule, LineEditInfoTest)
{
    m_passwordAuth->setLineEditInfo("Alert", AuthenticationModule::AlertText);
    m_passwordAuth->setLineEditInfo("Input", AuthenticationModule::InputText);
    m_passwordAuth->setLineEditInfo("PlaceHolder", AuthenticationModule::PlaceHolderText);
}
