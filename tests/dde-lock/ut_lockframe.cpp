#include "lockcontent.h"
#include "lockframe.h"
#include "lockworker.h"
#include "sessionbasemodel.h"

#include <QSignalSpy>

#include <gtest/gtest.h>

class UT_LockFrame : public testing::Test
{
protected:
    void SetUp();
    void TearDown();

    SessionBaseModel *m_sessionBaseModel;
    LockFrame *m_lockFrame;
    LockWorker *m_lockWorker;
};

void UT_LockFrame::SetUp()
{
    m_sessionBaseModel = new SessionBaseModel(SessionBaseModel::AuthType::LockType);
    m_lockFrame = new LockFrame(m_sessionBaseModel);
    m_lockWorker = new LockWorker(m_sessionBaseModel);
}

void UT_LockFrame::TearDown()
{
    delete m_sessionBaseModel;
    delete m_lockFrame;
    delete m_lockWorker;
}

TEST_F(UT_LockFrame, init)
{
    EXPECT_TRUE(m_lockFrame);
    m_lockWorker->onDisplayErrorMsg("aaaa");
    m_lockWorker->onDisplayTextInfo("ssssss");
    m_lockWorker->onPasswordResult("ddddd");
}
