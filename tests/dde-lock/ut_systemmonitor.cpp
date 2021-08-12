#include "systemmonitor.h"

#include <gtest/gtest.h>

class UT_SystemMonitor : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    SystemMonitor *m_systemMonitor;
};

void UT_SystemMonitor::SetUp()
{
    m_systemMonitor = new SystemMonitor;
}

void UT_SystemMonitor::TearDown()
{
    delete m_systemMonitor;
}

TEST_F(UT_SystemMonitor, BasicTest)
{
    m_systemMonitor->state();
    m_systemMonitor->setState(SystemMonitor::Enter);
}
