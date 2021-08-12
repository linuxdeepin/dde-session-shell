#include "loginbutton.h"

#include <gtest/gtest.h>

class UT_LoginButton : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    LoginButton *m_loginButton;
};

void UT_LoginButton::SetUp()
{
    m_loginButton = new LoginButton();
}

void UT_LoginButton::TearDown()
{
    delete m_loginButton;
}

TEST_F(UT_LoginButton, BasicTest)
{
    m_loginButton->setText("test");
    m_loginButton->setIcon("");
}
