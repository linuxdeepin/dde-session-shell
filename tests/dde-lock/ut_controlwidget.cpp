#include "controlwidget.h"
#include <gtest/gtest.h>
#include <QApplication>
#include <QKeyEvent>

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
    controlWidget->showTips();
    controlWidget->hideTips();
    controlWidget->rightKeySwitch();

    controlWidget->rightKeySwitch();
    controlWidget->keyReleaseEvent(new QKeyEvent(QEvent::KeyRelease, Qt::Key_Left, Qt::NoModifier));
    controlWidget->leftKeySwitch();
    controlWidget->keyReleaseEvent(new QKeyEvent(QEvent::KeyRelease, Qt::Key_Right, Qt::NoModifier));
    controlWidget->keyReleaseEvent(new QKeyEvent(QEvent::KeyRelease, Qt::Key_Exit, Qt::NoModifier));

    controlWidget->focusInEvent(nullptr);
    controlWidget->focusInEvent(nullptr);
    controlWidget->focusOutEvent(nullptr);
    controlWidget->focusOutEvent(nullptr);

    controlWidget->eventFilter(controlWidget->m_sessionBtn, new QEvent(QEvent::Enter));
    controlWidget->eventFilter(controlWidget->m_sessionBtn, new QEvent(QEvent::Leave));
    controlWidget->eventFilter(controlWidget->m_sessionTip, new QEvent(QEvent::Resize));

}
