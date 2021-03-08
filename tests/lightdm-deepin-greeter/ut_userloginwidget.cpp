#include <gtest/gtest.h>

#include <QTest>
#include <QLabel>

#include "userloginwidget.h"

class TstUserloginWidget : public testing::Test
{
public:
    void SetUp() override
    {
        m_userloginwidget = new UserLoginWidget();
    }
    void TearDown() override
    {
        delete m_userloginwidget;
        m_userloginwidget = nullptr;
    }
public:
    UserLoginWidget *m_userloginwidget = nullptr;
};

//TEST_F(TstUserloginWidget, coverage_main)
//{
//    ASSERT_NE(m_userloginwidget, nullptr);
//}

