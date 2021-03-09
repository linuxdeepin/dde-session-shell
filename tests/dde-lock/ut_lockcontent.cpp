#include "lockcontent.h"
#include "sessionbasemodel.h"

#include <gtest/gtest.h>

class UT_LockContent : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    LockContent *m_lockContent;
    SessionBaseModel *m_sessionBaseModel;
};

void UT_LockContent::SetUp()
{
    m_sessionBaseModel = new SessionBaseModel(SessionBaseModel::AuthType::LockType);
    m_lockContent = new LockContent(m_sessionBaseModel);
}

void UT_LockContent::TearDown()
{
    delete m_sessionBaseModel;
    delete m_lockContent;
}

TEST_F(UT_LockContent, init)
{
    EXPECT_TRUE(m_lockContent);
}
