#include "userloginwidget.h"
#include "authcommon.h"

#include <QLabel>
#include <QTest>
#include <QEvent>

#include <gtest/gtest.h>

using namespace AuthCommon;

class UT_UserloginWidget : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    UserLoginWidget *m_userloginwidget;
    SessionBaseModel *m_mode;
};

void UT_UserloginWidget::SetUp()
{
    m_mode = new SessionBaseModel(SessionBaseModel::AuthType::LightdmType);
    m_userloginwidget = new UserLoginWidget();
    m_userloginwidget->setModel(m_mode);
}
void UT_UserloginWidget::TearDown()
{
    delete m_userloginwidget;
    delete m_mode;
}

TEST_F(UT_UserloginWidget, init)
{
//     m_userloginwidget->initFingerprintAuth(0);
//     m_userloginwidget->initFaceAuth(0);
//     m_userloginwidget->initUkeyAuth(0);
//     m_userloginwidget->initFingerVeinAuth(0);
//     m_userloginwidget->initPINAuth(0);
//        EXPECT_TRUE(m_userloginwidget);

//        std::shared_ptr<NativeUser> nativeUser(new NativeUser("/com/deepin/daemon/Accounts/User"+QString::number((getuid()))));

//        m_userloginwidget->setName("aaaaa");

//        bool islogin = m_userloginwidget->getIsLogin();
//        m_userloginwidget->setIsLogin(!islogin);

//        bool isSelected = m_userloginwidget->getSelected();
//        m_userloginwidget->setSelected(!isSelected);
//        m_userloginwidget->setFastSelected(!isSelected);

//        bool isServer = m_userloginwidget->getIsServer();
//        m_userloginwidget->setIsServer(!isServer);

//        bool isServerMode = m_userloginwidget->getIsServerMode();
//        m_userloginwidget->setSelected(!isServerMode);

//        if (m_userloginwidget->getIsLogin()) {
//            m_userloginwidget->setWidgetShowType(UserLoginWidget::UserFrameLoginType);
//        } else {
//            m_userloginwidget->setWidgetShowType(UserLoginWidget::UserFrameType);
//        }

//        m_userloginwidget->setWidgetShowType(UserLoginWidget::WidgetShowType::NormalType);

//        if (m_userloginwidget->uid() == INT_MAX) {
//            m_userloginwidget->setWidgetShowType(UserLoginWidget::IDAndPasswordType);
//        } else {
//            if (nativeUser->isNoPasswdGrp()) {
//                m_userloginwidget->setWidgetShowType(UserLoginWidget::NoPasswordType);
//            } else {
//                m_userloginwidget->setWidgetShowType(UserLoginWidget::NormalType);
//            }
//        }
    m_userloginwidget->setWidgetType(UserLoginWidget::WidgetType::LoginType);
    std::shared_ptr<User> user_ptr(new User);
    m_userloginwidget->setUser(user_ptr);
    m_userloginwidget->setFaildTipMessage("Log in failed");
    m_userloginwidget->setSelected(true);
    m_userloginwidget->setFastSelected(true);
    m_userloginwidget->isSelected();
    m_userloginwidget->setUid(1);
    m_userloginwidget->uid();
    m_userloginwidget->ShutdownPrompt(SessionBaseModel::PowerAction::None);
    m_userloginwidget->updateAuthResult(AT_Password, AS_Success, "successful");
    m_userloginwidget->updateAvatar(user_ptr->avatar());
    m_userloginwidget->updateName(user_ptr->name());
    m_userloginwidget->updateLimitsInfo(user_ptr->limitsInfo());
    m_userloginwidget->updateAuthStatus();
    m_userloginwidget->clearAuthStatus();
    m_userloginwidget->updateLoginState(true);
    m_userloginwidget->updateNextFocusPosition();
    m_userloginwidget->updateAuthType(SessionBaseModel::AuthType::LockType);
    m_userloginwidget->unlockSuccessAni();
    m_userloginwidget->unlockFailedAni();
    //m_userloginwidget->updateAccoutLocale();
}

TEST_F(UT_UserloginWidget, event)
{
    //m_userloginwidget->focusOutEvent(new QFocusEvent(QEvent::Type::FocusOut));
    QTest::mousePress(m_userloginwidget, Qt::LeftButton, Qt::KeyboardModifier::NoModifier);
}
