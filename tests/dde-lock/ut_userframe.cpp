#include "sessionbasemodel.h"
#include "userframe.h"

#include <gtest/gtest.h>

class UT_UserFrame : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    UserFrame *m_userFrame;
    SessionBaseModel *m_model;
};

void UT_UserFrame::SetUp()
{
    m_userFrame = new UserFrame();
    m_model = new SessionBaseModel(SessionBaseModel::LockType);
}
void UT_UserFrame::TearDown()
{
    delete m_userFrame;
    delete m_model;
}

TEST_F(UT_UserFrame, basic)
{
    m_userFrame->setModel(m_model);
}
