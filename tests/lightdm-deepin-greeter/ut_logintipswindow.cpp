#include <gtest/gtest.h>

#include <QTest>
#include <QLabel>

#include "logintipswindow.h"

class TstLoginTipsWindow : public testing::Test
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

TEST_F(TstLoginTipsWindow, coverage_main)
{
    ASSERT_NE(loginTipsWindow, nullptr);
}

TEST_F(TstLoginTipsWindow, coverage_isValid)
{
    loginTipsWindow->isValid();
}
