#include "lockcontent.h"
#include "sessionbasemodel.h"

#include <gtest/gtest.h>

class UT_LockContent : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    SessionBaseModel *m_sessionBaseModel;
};

void UT_LockContent::SetUp()
{
    m_sessionBaseModel = new SessionBaseModel(SessionBaseModel::AuthType::LockType);
}

void UT_LockContent::TearDown()
{
    delete m_sessionBaseModel;
}

TEST_F(UT_LockContent, init)
{
}
