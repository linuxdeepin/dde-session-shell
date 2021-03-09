#include "timewidget.h"

#include <gtest/gtest.h>

class UT_TimeWidget : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    TimeWidget *timeWidget;
};

void UT_TimeWidget::SetUp()
{
    timeWidget = new TimeWidget();
}

void UT_TimeWidget::TearDown()
{
    delete timeWidget;
}

TEST_F(UT_TimeWidget, init)
{
}
