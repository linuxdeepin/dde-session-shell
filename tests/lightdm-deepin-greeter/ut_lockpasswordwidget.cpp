#include "lockpasswordwidget.h"

#include <gtest/gtest.h>
#include <QTest>
#include <QPaintEvent>

class UT_LockPasswordWidget : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    LockPasswordWidget *m_widget = nullptr;
};

void UT_LockPasswordWidget::SetUp()
{
    m_widget = new LockPasswordWidget();

}
void UT_LockPasswordWidget::TearDown()
{
    delete m_widget;
}


TEST_F(UT_LockPasswordWidget, init)
{
    m_widget->setMessage("");
    m_widget->paintEvent(new QPaintEvent(QRect()));
}
