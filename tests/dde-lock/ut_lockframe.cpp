#include <gtest/gtest.h>

#include "sessionbasemodel.h"
#include "lockframe.h"
#include "lockworker.h"
#include "lockcontent.h"

#include <QSignalSpy>

class UT_LockFrame : public testing::Test
{
public:
    void SetUp();
    void TearDown();

    SessionBaseModel *sessionBaseModel;
    LockFrame *lockFrame;
    LockWorker *lockWorker;
};

void UT_LockFrame::SetUp()
{
    sessionBaseModel = new SessionBaseModel(SessionBaseModel::AuthType::LockType);
    lockFrame = new LockFrame(sessionBaseModel);
    lockWorker = new LockWorker(sessionBaseModel);
}

void UT_LockFrame::TearDown()
{
    delete sessionBaseModel;
    delete lockFrame;
    delete lockWorker;
}

//TEST_F(UT_LockFrame, init)
//{
//    EXPECT_TRUE(1);
//}
