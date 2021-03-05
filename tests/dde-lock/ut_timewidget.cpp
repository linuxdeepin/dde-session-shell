#include <gtest/gtest.h>

#include "timewidget.h"

class UT_TimeWidget : public testing::Test
{
public:
    void SetUp() override;
    void TearDown() override;

protected:
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
