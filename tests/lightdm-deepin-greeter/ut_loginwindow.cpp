#include <gtest/gtest.h>

#include "sessionbasemodel.h"
#include "logincontent.h"
#include "greeterworkek.h"

#include <QSignalSpy>

class UT_LoginWindow : public testing::Test
{
public:
    void SetUp();
    void TearDown();

    SessionBaseModel *sessionBaseModel;
    LoginContent *loginContent;
    GreeterWorkek *greeterWorker;
};

void UT_LoginWindow::SetUp()
{
    sessionBaseModel = new SessionBaseModel(SessionBaseModel::AuthType::LockType);
    loginContent = new LoginContent(sessionBaseModel);
    greeterWorker = new GreeterWorkek(sessionBaseModel);
}

void UT_LoginWindow::TearDown()
{
    delete sessionBaseModel;
    delete loginContent;
    delete greeterWorker;
}

//TEST_F(UT_LoginWindow, init)
//{
//    EXPECT_TRUE(1);
//}
