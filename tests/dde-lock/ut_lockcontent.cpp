#include <gtest/gtest.h>

#include "sessionbasemodel.h"
#include "lockcontent.h"

class UT_LockContent : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    LockContent *lockContent;
    SessionBaseModel *sessionBaseModel;
};

void UT_LockContent::SetUp()
{
    sessionBaseModel = new SessionBaseModel(SessionBaseModel::AuthType::LockType);
    lockContent = new LockContent(sessionBaseModel);
}

void UT_LockContent::TearDown()
{
    delete sessionBaseModel;
    delete lockContent;
}

//TEST_F(UT_LockContent, init)
//{
//    EXPECT_TRUE(1);
//}
