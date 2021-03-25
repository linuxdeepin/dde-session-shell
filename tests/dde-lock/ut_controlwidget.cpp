#include "controlwidget.h"
#include <gtest/gtest.h>

class UT_ControlWidget : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    ControlWidget *controlWidget;
};

void UT_ControlWidget::SetUp()
{
    controlWidget = new ControlWidget();
}

void UT_ControlWidget::TearDown()
{
    delete controlWidget;
}

TEST_F(UT_ControlWidget, init)
{
    controlWidget->setVirtualKBVisible(true);
    controlWidget->setUserSwitchEnable(true);
    controlWidget->setSessionSwitchEnable(2<1);
    controlWidget->chooseToSession("aaaa");
}
