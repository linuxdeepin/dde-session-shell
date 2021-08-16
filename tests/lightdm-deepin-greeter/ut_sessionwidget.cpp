#include "sessionwidget.h"

#include <QLabel>
#include <QTest>

#include <gtest/gtest.h>

class TstSessionWidget : public testing::Test
{
public:
    void SetUp() override;
    void TearDown() override;

public:
    SessionWidget *m_sessionwidget;
};

void TstSessionWidget::SetUp()
{
    m_sessionwidget = new SessionWidget();
}

void TstSessionWidget::TearDown()
{
    delete m_sessionwidget;
}

TEST_F(TstSessionWidget, BasicTest)
{
    m_sessionwidget->sessionCount();
}
