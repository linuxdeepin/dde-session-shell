#include <gtest/gtest.h>

#include <QTest>
#include <QLabel>
#include <QSignalSpy>
#include <QPushButton>

#include "loginwindow.h"
#include "sessionbasemodel.h"

class UT_LoginWindow : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    SessionBaseModel *m_sessionBaseModel;
    LoginWindow *m_loginwindow;
};

void UT_LoginWindow::SetUp()
{
    m_sessionBaseModel = new SessionBaseModel(SessionBaseModel::AuthType::LightdmType);
    m_loginwindow = new LoginWindow(m_sessionBaseModel);

}

void UT_LoginWindow::TearDown()
{
    delete m_sessionBaseModel;
    delete m_loginwindow;
}

TEST_F(UT_LoginWindow, coverage_main)
{
    ASSERT_NE(m_loginwindow, nullptr);
}

TEST_F(UT_LoginWindow, coverage_isValid)
{

}

TEST_F(UT_LoginWindow, buttonClicked)
{


}
