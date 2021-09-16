#include "controlwidget.h"
#include <gtest/gtest.h>
#include <QApplication>
#include <QKeyEvent>

#include <QTest>

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
    controlWidget->setUserSwitchEnable(false);
    controlWidget->setSessionSwitchEnable(true);
    controlWidget->chooseToSession("");
    controlWidget->chooseToSession("aaaa");
    controlWidget->rightKeySwitch();

    controlWidget->rightKeySwitch();
    controlWidget->leftKeySwitch();
    controlWidget->showTips();
    controlWidget->hideTips();
    QTest::keyRelease(controlWidget, Qt::Key_0, Qt::KeyboardModifier::NoModifier);
}
