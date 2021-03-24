#include "userloginwidget.h"

#include <gtest/gtest.h>
#include <QTest>
#include <QLabel>

class UT_UserloginWidget : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    UserLoginWidget *m_userloginwidget = nullptr;
};

void UT_UserloginWidget::SetUp()
{
    // m_userloginwidget = new UserLoginWidget();
}
void UT_UserloginWidget::TearDown()
{
    delete m_userloginwidget;
    m_userloginwidget = nullptr;
}


TEST_F(UT_UserloginWidget, init)
{
//    EXPECT_TRUE(m_userloginwidget);

//    std::shared_ptr<NativeUser> nativeUser(new NativeUser("/com/deepin/daemon/Accounts/User"+QString::number((getuid()))));

//    m_userloginwidget->setName("aaaaa");

//    bool islogin = m_userloginwidget->getIsLogin();
//    m_userloginwidget->setIsLogin(!islogin);

//    bool isSelected = m_userloginwidget->getSelected();
//    m_userloginwidget->setSelected(!isSelected);
//    m_userloginwidget->setFastSelected(!isSelected);

//    bool isServer = m_userloginwidget->getIsServer();
//    m_userloginwidget->setIsServer(!isServer);

//    bool isServerMode = m_userloginwidget->getIsServerMode();
//    m_userloginwidget->setSelected(!isServerMode);


//    if (m_userloginwidget->getIsLogin()) {
//        m_userloginwidget->setWidgetShowType(UserLoginWidget::UserFrameLoginType);
//    } else {
//        m_userloginwidget->setWidgetShowType(UserLoginWidget::UserFrameType);
//    }

//    m_userloginwidget->setWidgetShowType(UserLoginWidget::WidgetShowType::NormalType);


//    if (m_userloginwidget->uid() == INT_MAX) {
//        m_userloginwidget->setWidgetShowType(UserLoginWidget::IDAndPasswordType);
//    } else {
//        if (nativeUser->isNoPasswdGrp()) {
//            m_userloginwidget->setWidgetShowType(UserLoginWidget::NoPasswordType);
//        } else {
//            m_userloginwidget->setWidgetShowType(UserLoginWidget::NormalType);
//        }
//    }

}
