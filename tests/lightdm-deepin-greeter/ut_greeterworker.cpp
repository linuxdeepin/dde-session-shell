#include "greeterworkek.h"
#include "sessionbasemodel.h"

#include <gtest/gtest.h>

class UT_GreeterWorker : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    GreeterWorkek *m_worker;
    SessionBaseModel *m_model;
};

void UT_GreeterWorker::SetUp()
{
    m_model = new SessionBaseModel(SessionBaseModel::LightdmType);
    m_worker = new GreeterWorkek(m_model);
}

void UT_GreeterWorker::TearDown()
{
    delete m_worker;
    delete m_model;
}

TEST_F(UT_GreeterWorker, BasicTest)
{
    std::shared_ptr<User> user_ptr(new User);
    m_worker->switchToUser(user_ptr);

    m_worker->onDisplayErrorMsg("test");
    m_worker->onDisplayTextInfo("test");
    m_worker->onPasswordResult("test");
}

TEST_F(UT_GreeterWorker, AuthTest)
{
    const QString UserName("uos");
    m_worker->checkAccount(UserName);
//    m_worker->createAuthentication(UserName);
//    m_worker->startAuthentication(UserName, 19);
//    m_worker->sendTokenToAuth(UserName, 1, "123");
//    m_worker->endAuthentication(UserName, 19);
//    m_worker->destoryAuthentication(UserName);
    m_worker->restartResetSessionTimer();
    m_worker->doPowerAction(SessionBaseModel::PowerAction::RequireLock);
    std::shared_ptr<User> user_ptr(new User);
    m_worker->setCurrentUser(user_ptr);
    m_worker->oneKeyLogin();
    m_worker->userAuthForLightdm(user_ptr);
    m_worker->showPrompt("", QLightDM::Greeter::PromptType::PromptTypeQuestion);
    m_worker->showMessage("", QLightDM::Greeter::MessageType::MessageTypeInfo);
    m_worker->authenticationComplete();
    m_worker->saveNumlockStatus(user_ptr, false);
    m_worker->recoveryUserKBState(user_ptr);
    m_worker->resetLightdmAuth(user_ptr, 0, false);
}
