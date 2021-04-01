#include <gtest/gtest.h>

#include <QTest>
#include <QLabel>
#include <QSignalSpy>
#include <QPushButton>

#include "logintipswindow.h"

class UT_LoginTipsWindow : public testing::Test
{
public:
    void SetUp() override
    {
        loginTipsWindow = new LoginTipsWindow();
    }
    void TearDown() override
    {
        delete loginTipsWindow;
        loginTipsWindow = nullptr;
    }
public:
    LoginTipsWindow *loginTipsWindow = nullptr;
};

TEST_F(UT_LoginTipsWindow, coverage_main)
{
    ASSERT_NE(loginTipsWindow, nullptr);
}

TEST_F(UT_LoginTipsWindow, coverage_isValid)
{
    loginTipsWindow->isValid();
}

TEST_F(UT_LoginTipsWindow, buttonClicked)
{
    QPushButton button1;
    QSignalSpy spy(&button1, SIGNAL(clicked()));
    QPushButton *button = loginTipsWindow->findChild<QPushButton *>("RequireSureButton");
    QTest::mouseClick(button, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(0, 0));

}
