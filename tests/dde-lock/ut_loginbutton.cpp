#include "loginbutton.h"
#include <QEvent>
#include <QTest>

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
    //m_loginButton->enterEvent(new QEvent(QEvent::Type::Enter));
    //m_loginButton->leaveEvent(new QEvent(QEvent::Type::Leave));
    //m_loginButton->paintEvent(new QPaintEvent(m_loginButton->rect()));
    QTest::mousePress(m_loginButton, Qt::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(0, 0));
    QTest::mouseRelease(m_loginButton, Qt::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(0, 0));
    QTest::keyPress(m_loginButton, Qt::Key_0, Qt::KeyboardModifier::NoModifier);
}
