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
    m_userloginwidget = new UserLoginWidget();
}
void UT_UserloginWidget::TearDown()
{
    delete m_userloginwidget;
    m_userloginwidget = nullptr;
}

//TEST_F(TstUserloginWidget, coverage_main)
//{
//    ASSERT_NE(m_userloginwidget, nullptr);
//}

TEST_F(UT_UserloginWidget, init)
{
    EXPECT_TRUE(m_userloginwidget);

    bool islogin = m_userloginwidget->getIsLogin();
    m_userloginwidget->setIsLogin(!islogin);

}
